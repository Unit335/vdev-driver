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
vd_devices.ko создает устройства, vd_drv.ko выполняет функции драйвера.

make test собирает программу test с небольшим набором команд для тестирования интерфейса /dev/vdev-X



