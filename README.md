# vdev-driver
## Описание

Два модуля ядра, **vd_devices** эмулирует подключение 4х устройств размером 4096 байт, **vd_driver** предоставляет выделяет для них память и создает следующие интерфейсы:
write/read/open/mmap/llseek через /dev/vdev-X (в данном случае устройства четыре, X от 0 до 3);
sysfs: через /sys/class/vd_class/vdev-X посредством файла address (используется для записи позиции относительно начала файла) и value, через который производится чтение данных по сдвигу из address.

## Сборка и запуск

Сборка и включение необходимых модулей
```sh
make
make enable
```
Отключение
```sh
make disable
```
**vd_devices.ko** создает устройства, **vd_drv.ko** выполняет функции драйвера.

**make test** собирает программу test с небольшим набором команд для тестирования интерфейса /dev/vdev-X

## Интерфейс /dev/vdev-X
Через /dev/vdev-X можно взаимодействовать с памятью устройства как с файлом, например:

```sh
~$ echo "devtest10932" | sudo tee /dev/vdev-0
devtest10932
~$ sudo cat /dev/vdev-0 
devtest10932
```

Программа test также реализует взаимодействие с /dev/vdev-1 для записи и чтения отдельных байтов файла. Варианты write и read проводят запись и чтение одного байта данных, по умолчанию после каждой операции производится сдвиг текущей позиции в файле на количество записанных/прочитанных (в данном случае 1) байт, для отдельного управления позицией в файле необходимо использовать вариант lseek, устанавливающий позицию относительно начала файла. 

## Интерфейс sysfs

Данные устройств расположены в /sys/class/vd_class в папках вида vdev-X. Атрибуты address и value в папке устройства (например, в /sys/class/vd_class/vdev-1) указывают на сдвиг в байтах относительно начала данных устройств и значение (для отдельных байтов - в формате unsigned int) по этому сдвигу соответственно.  
address по умолчанию имеет значение 0. 

Например:
```sh
~$ echo "devtest1234 5 6" | sudo tee /dev/vdev-1
devtest1234 5 6
~$ sudo cat /dev/vdev-1
devtest1234 5 6

~$ sudo cat /sys/class/vd_class/vdev-1/value 
100
~$ sudo cat /sys/class/vd_class/vdev-1/address
0

~$ echo 1 | sudo tee /sys/class/vd_class/vdev-1/address 
1
~$ sudo cat /sys/class/vd_class/vdev-1/value 
101

~$ echo 100 | sudo tee /sys/class/vd_class/vdev-1/value 
100
~$ sudo cat /dev/vdev-1
devtest123d5 6
```

## mmap
mmap позволяет непосредственно работать с памятью устройства из программы. В test.c реализовано чтение данных /dev/vdev-1 с последующей записью новой строки.
```
~$ sudo ./test 
Options: 
1 - write
2 - read
3 - lseek
4 - mmap
5 - exit
4
Choose option = 4

Current file data:
devtest123d

Wrote 32 bytes of new data:
Test-test-0987654321!@#$%%&*///
```

Чтение новых данных из /dev/vdev-1:
```sh
~$ sudo cat /dev/vdev-1
Test-test-0987654321!@#$%%&*///
```
