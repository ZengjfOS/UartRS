#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>      
#include <termios.h>    
#include <errno.h>  
#include <pthread.h>
#include <string.h>

#define SENDWORKMODE        1
#define RECVWORKMODE        2
#define SENDANDRECVWORKMODE 3
#define LOOPBACKTest        4

int  serial_fd          = 0;  
int  serialNumber       = 0;
int  baudRate           = 0;
int  workMode           = 0;        // 1 send; 2 recv; 3 send and recv; 4 loop back
char serialString[20]   = {0};

char sendString[512]    = {0};
char recvString[512]    = {0};

int  help ( int argc );
int  initSerial ( void );

int  getBaudRate ( char *baudRateString );
void getSerial ( char* commandLineserialString );
int  getWorkMode ( char *workMode );

int  uart_send ( int fd, char *data, int datalen );
int  uart_recv ( int fd, char *data, int datalen );

pthread_t recv_thread;
void *recvDataThread ( void *arg );

pthread_t quitOut_thread;
void *quitOutThead ( void *arg );

void* thread_ret;

int main(int argc, char **argv)  
{  
    
    if ( help( argc ) != 0)
        return -1;
    
    getSerial ( argv[1] );
    baudRate = getBaudRate ( argv[2] );
    workMode = getWorkMode ( argv[3] );

    initSerial ();

    if ( workMode == RECVWORKMODE || workMode == SENDANDRECVWORKMODE || workMode == LOOPBACKTest ) {
        pthread_create ( &recv_thread, NULL, recvDataThread, NULL );
    }

    if ( workMode != LOOPBACKTest ) 
        pthread_create ( &quitOut_thread, NULL, quitOutThead, NULL );

    int i = 0;
    while ( 1 ) {

        if ( workMode == LOOPBACKTest ) 
            usleep ( 100000 );

        if ( workMode == SENDWORKMODE || workMode == SENDANDRECVWORKMODE || workMode == LOOPBACKTest ) {

            if ( workMode != LOOPBACKTest ) 
                sprintf ( sendString, "%03d: %s\r\n", i++, argv[4] );
            else
                sprintf ( sendString, "%s\r\n", argv[4] );

            uart_send ( serial_fd, sendString, strlen ( sendString ) );  
        }
        if ( workMode == LOOPBACKTest ) 
            break;

        usleep ( 2000000 );
    } 

    pthread_join(recv_thread, &thread_ret);
      
    close ( serial_fd );  
    exit(0);
}

int getWorkMode ( char *workModeString ) {

    int ret = atoi( workModeString );

    switch ( ret ) {
        case 1 :
            printf ( "workMode: send.\n" );
            break;
        case 2 :
            printf ( "workMode: recv.\n" );
            break;
        case 3 :
            printf ( "workMode: send and recv.\n" );
            break;
        case 4 :
            printf ( "workMode: loopback test.\n" );
            break;
        default:
            printf ( "none of this workMode.\n" );
            exit ( 0 );
    }

    return ret;
}

void *quitOutThead(void *arg) {

    //system("stty raw -echo");     can't work well in Android linux
    char ch = '\0';

    while ( 1 ) {
        scanf ( "%c", &ch );
        if ( ch == 'q' || ch == 'Q' ) {
            exit ( 0 );
        }
    }
}

void *recvDataThread(void *arg) {

    int ret = 0;
    int i = 0;

    while ( 1 ) {
        ret = uart_recv ( serial_fd, recvString, sizeof(recvString) );
        //printf ( "%03d %s\n", i++, recvString );
        //printf ( "%03d %s", 0, recvString );

        printf ( "%s", recvString );

        if ( workMode == LOOPBACKTest )
            if (strchr(recvString, '\n'))
                break;

        bzero ( recvString, sizeof(recvString) );
        usleep ( 200000 );
    }

    return 0;
}

int help( int argc ) {

    if ( argc != 5 ) {
        printf ( "USAGE:\n" );
        printf ( "    command <serial absolute path> <baudRate> <workMode> <send String>\n" );
        printf ( "    example:\n" );
        printf ( "        ./uartRS /dev/ttymxc1 115200 3 \"1234567890ABCDEFG\"\n" );
        return -1;
    }

    return 0;
}

int uart_recv(int fd, char *data, int datalen) {  

    int ret = 0;

    ret = read ( fd, data, datalen );
      
    return ret;  
}

int uart_send(int fd, char *data, int datalen) {  

    int len = 0;  

    len = write ( fd, data, datalen );     //实际写入的长度  
    if(len == datalen) {  
        return len;  
    } else {  
        tcflush(fd, TCOFLUSH);          //TCOFLUSH刷新写入的数据但不传送  
        return -1;  
    }  
      
    return 0;  
}  

int initSerial( void ) {  

    //serial_fd = open( serialString, O_RDWR | O_NOCTTY | O_NDELAY );  
    serial_fd = open ( serialString, O_RDWR );  
    if ( serial_fd < 0 ) {  
        perror ( "open" );  
        return -1;  
    }  
      
    // 串口主要设置结构体termios <termios.h>  
    struct termios options;  
      
    /**
     * tcgetattr函数用于获取与终端相关的参数，参数fd为终端的文件描述符，
     * 返回的结果保存在termios结构体中 
     */  
    tcgetattr ( serial_fd, &options );  
    /**2. 修改所获得的参数*/  
    options.c_cflag |= (CLOCAL | CREAD);    //设置控制模式状态，本地连接，接收使能  
    options.c_cflag &= ~CSIZE;              //字符长度，设置数据位之前一定要屏掉这个位  
    options.c_cflag &= ~CRTSCTS;            //无硬件流控  
    options.c_cflag |= CS8;                 //8位数据长度  
    options.c_cflag &= ~CSTOPB;             //1位停止位  
    options.c_iflag |= IGNPAR;              //无奇偶检验位  
    options.c_oflag = 0;                    //输出模式  
    options.c_lflag = 0;                    //不激活终端模式  
    cfsetospeed ( &options, baudRate );        //设置波特率  
    //cfsetospeed(&options, B2000000);//设置波特率  
      
    /**3. 设置新属性，TCSANOW：所有改变立即生效*/  
    tcflush ( serial_fd, TCIFLUSH );           //溢出数据可以接收，但不读  
    tcsetattr ( serial_fd, TCSANOW, &options );  
      
    return 0;  
}  

void getSerial ( char* commandLineserialString ) {

    sprintf ( serialString, "%s", commandLineserialString );
    printf ( "serialString : %s.\n", serialString );
}

int getBaudRate ( char* baudRateString ) {
    int ret = atoi ( baudRateString );
    switch ( ret ) {
        case 0:
            printf ( "baudRate %s.\n", "0" );
            ret = B0;
            break;
        case 50:
            printf ( "baudRate %s.\n", "50" );
            ret = B50;
            break;
        case 75:
            printf ( "baudRate %s.\n", "75" );
            ret = B75;
            break;
        case 110:
            printf ( "baudRate %s.\n", "110" );
            ret = B110;
            break;
        case 134:
            printf ( "baudRate %s.\n", "134" );
            ret = B134;
            break;
        case 150:
            printf ( "baudRate %s.\n", "150" );
            ret = B150;
            break;
        case 200:
            printf ( "baudRate %s.\n", "200" );
            ret = B200;
            break;
        case 300:
            printf ( "baudRate %s.\n", "300" );
            ret = B300;
            break;
        case 600:
            printf ( "baudRate %s.\n", "600" );
            ret = B600;
            break;
        case 1200:
            printf ( "baudRate %s.\n", "1200" );
            ret = B1200;
            break;
        case 1800:
            printf ( "baudRate %s.\n", "1800" );
            ret = B1800;
            break;
        case 2400:
            printf ( "baudRate %s.\n", "2400" );
            ret = B2400;
            break;
        case 4800:
            printf ( "baudRate %s.\n", "4800" );
            ret = B4800;
            break;
        case 9600:
            printf ( "baudRate %s.\n", "9600" );
            ret = B9600;
            break;
        case 19200:
            printf ( "baudRate %s.\n", "19200" );
            ret = B19200;
            break;
        case 38400:
            printf ( "baudRate %s.\n", "38400" );
            ret = B38400;
            break;
        case 57600:
            printf ( "baudRate %s.\n", "57600" );
            ret = B57600;
            break;
        case 115200:
            printf ( "baudRate %s.\n", "115200" );
            ret = B115200;
            break;
        case 230400:
            printf ( "baudRate %s.\n", "230400" );
            ret = B230400;
            break;
        case 460800:
            printf ( "baudRate %s.\n", "460800" );
            ret = B460800;
            break;
        case 500000:
            printf ( "baudRate %s.\n", "500000" );
            ret = B500000;
            break;
        case 576000:
            printf ( "baudRate %s.\n", "576000" );
            ret = B576000;
            break;
        case 921600:
            printf ( "baudRate %s.\n", "921600" );
            ret = B921600;
            break;
        case 1000000:
            printf ( "baudRate %s.\n", "1000000" );
            ret = B1000000;
            break;
        case 1152000:
            printf ( "baudRate %s.\n", "1152000" );
            ret = B1152000;
            break;
        case 1500000:
            printf ( "baudRate %s.\n", "1500000" );
            ret = B1500000;
            break;
        case 2000000:
            printf ( "baudRate %s.\n", "2000000" );
            ret = B2000000;
            break;
        case 2500000:
            printf ( "baudRate %s.\n", "2500000" );
            ret = B2500000;
            break;
        case 3000000:
            printf ( "baudRate %s.\n", "3000000" );
            ret = B3000000;
            break;
        case 3500000:
            printf ( "baudRate %s.\n", "3500000" );
            ret = B3500000;
            break;
        case 4000000:
            printf ( "baudRate %s.\n", "4000000" );
            ret = B4000000;
            break;
        default:
            printf ( "baudRate is not exist %s.\n", "0" );
            ret = B0;
    }
    //printf ("baudRate %s.\n", baudRateString);
    return ret;
}
