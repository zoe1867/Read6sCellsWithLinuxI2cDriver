#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#define I2C_ADDR 0x0b

#define RESET_CURSOR() printf("\033[H")
#define CLEAR() printf("\033[2J")

unsigned short I2C_SF_ChkSum(unsigned char *pData,unsigned char count)
{
    unsigned short ChkSum = 0;
    while(count--)
    {
        ChkSum += (*pData++); 
    }
    return (ChkSum);
}

int main(void)
{
    int fd;

    unsigned char WrBuf[5]={0xf0,0x01,0x00,0x02,0x00};
    unsigned char ChkVal = 0;
    unsigned char RdBuf[200];
    unsigned short BmsTotalVolt = 0;
    unsigned short CellVolt[6];
    unsigned short Rsoc = 0;
    unsigned short Soc[6];
    short BmsCurrent = 0;
    short BmsTemperature = 0;
    unsigned short CycleCnt = 0;
    unsigned char DatFielLen = 0;
    unsigned char i = 0;
    int PackNo = 0;

    fd=open("/dev/i2c-1",O_RDWR);
    if(fd<0)
    {
        printf("err open file:%s\r\n",strerror(errno)); 
        return 1;
    }
    if(ioctl( fd,I2C_SLAVE,I2C_ADDR)<0 )
    {
        printf("ioctl error : %s\r\n",strerror(errno));
        return 1;
    }


    while(1)
    {
        printf("***********Pack No: %d \r\n",PackNo);
        PackNo++;
        //total voltage, and vcell1~6
        WrBuf[2] = 0x00;
        DatFielLen = 14;
        WrBuf[3] = DatFielLen;
        ChkVal = (unsigned char)I2C_SF_ChkSum(WrBuf,4);
        WrBuf[4] = ChkVal;

        if(write(fd,&WrBuf,5)<0)
            printf("write 0x01 err\r\n");      
        usleep(1000);

        if(read(fd,&RdBuf,DatFielLen+4))  
        {
            BmsTotalVolt = RdBuf[3]+RdBuf[4]*256;
            BmsTotalVolt = BmsTotalVolt*10;
            printf("0x00 TotalVolt is %d\r\n",BmsTotalVolt);
            for(i=0;i<6;i++)
            {
                CellVolt[i] = RdBuf[5+i*2]+RdBuf[6+i*2]*256;
                CellVolt[i] = CellVolt[i] *10;
                printf("Cell[%d] is %d\r\n",i+1,CellVolt[i]);
            }
            
        }
        else
            printf("read error\r\n");
        //end of total voltage

        //soc, and soc1~6
        usleep(5000);
        WrBuf[2] = 0x20;
        DatFielLen = 14;
        WrBuf[3] = DatFielLen;
        ChkVal = (unsigned char)I2C_SF_ChkSum(WrBuf,4);
        WrBuf[4] = ChkVal;
        if(write(fd,&WrBuf,5)<0)
            printf("write 0x01 err\r\n");       
        usleep(1000);

        if(read(fd,&RdBuf,DatFielLen+4))  
        {
            Rsoc = RdBuf[3]+RdBuf[4]*256;
            Rsoc = Rsoc/10;
            printf("0x20 Roc is %d\r\n",Rsoc);
            for(i=0;i<6;i++)
            {
                Soc[i] = RdBuf[5+i*2]+RdBuf[6+i*2]*256;
                Soc[i] = Soc[i] /10;
                printf("Cell[%d] is %d\r\n",i+1,Soc[i]);
            }
            
        }
        else
            printf("read error\r\n");

        //current
        usleep(5000);
        WrBuf[2] = 0x40;
        DatFielLen = 2;
        WrBuf[3] = DatFielLen;
        ChkVal = (unsigned char)I2C_SF_ChkSum(WrBuf,4);
        WrBuf[4] = ChkVal;
        if(write(fd,&WrBuf,5)<0)
            printf("write 0x01 err\r\n");     
        usleep(1000);

        if(read(fd,&RdBuf,DatFielLen+4))  
        {
            BmsCurrent = RdBuf[3]+RdBuf[4]*256;
            BmsCurrent = Rsoc*10;
            printf("0x40 Current is %d\r\n",BmsCurrent);         
        }
        else
            printf("read error\r\n");

        //Temp
        usleep(5000);
        WrBuf[2] = 0xa0;
        DatFielLen = 2;
        WrBuf[3] = DatFielLen;
        ChkVal = (unsigned char)I2C_SF_ChkSum(WrBuf,4);
        WrBuf[4] = ChkVal;
        if(write(fd,&WrBuf,5)<0)
            printf("write 0x01 err\r\n");    
        usleep(1000);

        if(read(fd,&RdBuf,DatFielLen+4))  
        {
            BmsTemperature = RdBuf[3]+RdBuf[4]*256;
            BmsTemperature = BmsTemperature/10;
            printf("0xa0 Temperature is %d\r\n",BmsTemperature);         
        }
        else
            printf("read error\r\n");

        //Cycle Count
        usleep(5000);
        WrBuf[2] = 0xa2;
        DatFielLen = 2;
        WrBuf[3] = DatFielLen;
        ChkVal = (unsigned char)I2C_SF_ChkSum(WrBuf,4);
        WrBuf[4] = ChkVal;
        if(write(fd,&WrBuf,5)<0)
            printf("write 0x01 err\r\n");    
        usleep(1000);

        if(read(fd,&RdBuf,DatFielLen+4))  
        {
            CycleCnt = RdBuf[3]+RdBuf[4]*256;
            printf("0xa2 CycleCount is %d\r\n",CycleCnt);         
        }
        else
            printf("read error\r\n");

        usleep(1000000);//delay 1s
        CLEAR();//clear 
        RESET_CURSOR();//reset cursor           
    }
}