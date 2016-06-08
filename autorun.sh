# ./uartRS <serial absolute path> <baudRate> <work mode> <send String>
#   1.serial absolute path
#     the absolute path for serial port, example:
#         /dev/ttymxc1
#   2. referrence baudrate:
#      0
#      50
#      75
#      110
#      134
#      150
#      200
#      300
#      600
#      1200
#      1800
#      2400
#      4800
#      9600
#      19200
#      38400
#      57600
#      115200
#      230400
#      460800
#      500000
#      576000
#      921600
#      1000000
#      1152000
#      1500000
#      2000000
#      2500000
#      3000000
#      3500000
#      4000000
#   3. work mode
#     1. send;
#     2. recv;
#     3. send and recv
#   4. send string
#     the string what you want to send

chmod 777 ./uartRS              # give excute-permission
# ./uartRS <serial absolute path> <baudRate> <work mode> <send String>
#./uartRS      /dev/ttymxc2          9600        3       "1234567890ABCDEF"
#./uartRS      /dev/ttymxc2          19200        3       ""
#./uartRS      /dev/ttymxc1          9600        2       "AT+CTSP=2,0,0"
#./uartRS      /dev/ttyACM0          9600        3       "AT+UUSBCONF?"
#sleep 2 
./uartRS      /dev/ttyACM0          9600        3       "AT+UUSBCONF=0,\"\",0"
#./uartRS      /dev/ttyACM0          9600        3       "AT+CFUN=16"
#sleep 20 
#./uartRS      /dev/ttyACM0          9600        3       "AT+UUSBCONF?"
#sleep 20 
#./uartRS      /dev/ttyACM0          9600        3       "AT+UUSBCONF?"
