/*

    2022/11/26
    - could communication QPSK
    - delete buffer memory
        - use new and delete
    - recieve "dataFmt" -> https://discourse.myriadrf.org/t/data-format-of-samples/6487/4

*/


#include <iostream>
#include "lime/LimeSuite.h"
#include <thread>
#include <chrono>
#include <math.h>
#include <complex.h>
#include<fstream>

using namespace std;

lms_device_t* device;
const int frequency = 920e6;
const double sampleRate = 0.2e6;
const int streamTime = 20;
const int fifoSize = 256;
bool runningTx1;
bool runningTx2;
bool running;
bool start = false;

int error() {
  cout << "ERROR: " << LMS_GetLastErrorMessage() << endl;
  LMS_Close(device);
  exit(-1);
}

void StreamTx1() {
    std::ifstream in_stream_re;
    in_stream_re.open("input_re_include_zero.dat");
    std::ifstream in_stream_im;
    in_stream_im.open("input_im_include_zero.dat"); 
    if(in_stream_re.fail() || in_stream_im.fail()){	
		std::cerr << "ファイルを開けません\n";
		exit(1);
	}

    int array_size = 1024*16;
    int linenum = 0; // データの行数を数える
    char* buf_re = new char[array_size];
    char* buf_im = new char[array_size];
    while(in_stream_re.getline(buf_re, sizeof(buf_re))){	// ファイルから1行ずつ読み込む
		linenum++;	// 行数をカウントしている
	}
    std::cerr << "number line of datFile = " << linenum << "\n";

    in_stream_re.clear(); // ファイル末尾に到達というフラグをクリア
	in_stream_re.seekg(0, std::ios::beg);	// ファイル先頭に戻る

    array_size = linenum; // 動的に配列のサイズを確保　いるか？？
    int tx_size = linenum; // should be same size of inputed matlab data
    float* tx_buffer = new float[2*tx_size]; // TODO: think about tx.fifo

	for(int i=0 ; i<tx_size ; i++){
		in_stream_re.getline(buf_re,sizeof(buf_re));	// 一行読み込んで…
		in_stream_im.getline(buf_im,sizeof(buf_im));
		tx_buffer[2*i] = atoi(buf_re);	// それを配列に格納
		tx_buffer[2*i+1] = atoi(buf_im);
		// tx_buffer[2*i] = 1;
		// tx_buffer[2*i+1] = 0;
        // printf("%.1f %+.1fi\n", tx_buffer[2*i], tx_buffer[2*i+1]);
	}

    // want to clear buf_re
    in_stream_re.close();
    delete[] buf_re;
    delete[] buf_im;

    lms_stream_t tx_stream1;
    tx_stream1.channel = 0;
    tx_stream1.fifoSize = fifoSize;
    tx_stream1.throughputVsLatency = 0.5;
    tx_stream1.dataFmt = lms_stream_t::LMS_FMT_F32;
    tx_stream1.isTx = true;
    lms_stream_meta_t meta_tx1;   
    meta_tx1.waitForTimestamp = false;
    meta_tx1.flushPartialPacket = false;
    meta_tx1.timestamp = 0;

    if (LMS_SetupStream(device, &tx_stream1) != 0)
        error();

    LMS_StartStream(&tx_stream1);

    while (!start) {
        // waiting GPIO pin HIGH
    }
    printf("start TX1!!\n");
    
    int runningNum = 0;
    while (runningTx1) {
        runningNum = runningNum + 1;
        int ret1 = LMS_SendStream(&tx_stream1,tx_buffer,tx_size,&meta_tx1,1000);
        if (ret1 != tx_size)
            cout << "error: samples sent: " << ret1 << "/" << tx_size << endl;;
    }   
    
    printf("finished TX1\n");
    printf("runningNum: %d\n", runningNum); // runningtime 5s: fifo=256 -> 172, fifo=1024 -> 172, fifo=1024*1024 -> 348, fifo=1024*1024*4 -> 872, fifo=1024*1024*256 -> 44832
    delete[] tx_buffer;

    LMS_StopStream(&tx_stream1);
    LMS_DestroyStream(device,&tx_stream1);
}

void StreamTx2() {
    std::ifstream in_stream_re;
    in_stream_re.open("input_re_include_zero.dat");
    std::ifstream in_stream_im;
    in_stream_im.open("input_im_include_zero.dat"); 
    if(in_stream_re.fail() || in_stream_im.fail()){	
		std::cerr << "ファイルを開けません\n";
		exit(1);
	}

    int array_size = 1024*16;
    int linenum = 0; // データの行数を数える
    char* buf_re = new char[array_size];
    char* buf_im = new char[array_size];
    while(in_stream_re.getline(buf_re, sizeof(buf_re))){	// ファイルから1行ずつ読み込む
		linenum++;	// 行数をカウントしている
	}
    std::cerr << "number line of datFile = " << linenum << "\n";

    in_stream_re.clear(); // ファイル末尾に到達というフラグをクリア
	in_stream_re.seekg(0, std::ios::beg);	// ファイル先頭に戻る

    array_size = linenum; // 動的に配列のサイズを確保　いるか？？
    int tx_size = linenum; // should be same size of inputed matlab data
    float* tx_buffer = new float[2*tx_size]; // TODO: think about tx.fifo

	for(int i=0 ; i<tx_size ; i++){
		in_stream_re.getline(buf_re,sizeof(buf_re));	// 一行読み込んで…
		in_stream_im.getline(buf_im,sizeof(buf_im));
		tx_buffer[2*i] = atoi(buf_re);	// それを配列に格納
		tx_buffer[2*i+1] = atoi(buf_im);
		// tx_buffer[2*i] = 1;
		// tx_buffer[2*i+1] = 0;
        // printf("%.1f %+.1fi\n", tx_buffer[2*i], tx_buffer[2*i+1]);
	}

    // want to clear buf_re
    in_stream_re.close();
    delete[] buf_re;
    delete[] buf_im;

    lms_stream_t tx_stream2;
    tx_stream2.channel = 1;
    tx_stream2.fifoSize = fifoSize;
    tx_stream2.throughputVsLatency = 0.5;
    tx_stream2.dataFmt = lms_stream_t::LMS_FMT_F32;
    tx_stream2.isTx = true;
    lms_stream_meta_t meta_tx2;   
    meta_tx2.waitForTimestamp = false;
    meta_tx2.flushPartialPacket = false;
    meta_tx2.timestamp = 0;
    if (LMS_SetupStream(device, &tx_stream2) != 0)
        error();

    LMS_StartStream(&tx_stream2);
    
    while (!start) {
        // waiting GPIO pin HIGH
    }
    printf("start TX2!!\n");
    
    while (runningTx2) {
        int ret2 = LMS_SendStream(&tx_stream2,tx_buffer,tx_size,&meta_tx2,1000);
        if (ret2 != tx_size)
            cout << "error: samples sent: " << ret2 << "/" << tx_size << endl;;
    }   
    
    printf("finished TX2\n");
    delete[] tx_buffer;

    LMS_StopStream(&tx_stream2);
    LMS_DestroyStream(device,&tx_stream2); 
}

void print_gpio(int gpio_val)
{
    for (int i = 0; i < 8; i++)
    {
        bool set = gpio_val&(1<<i); 
        std::cout << "GPIO" << i <<": " << (set ? "High" : "Low") << std::endl;
    }
}


int main(int argc, char** argv)
{
    int n= LMS_GetDeviceList(nullptr);
    if (n > 0)
    {
        if (LMS_Open(&device,NULL,NULL)!=0) //open first device
            error();

        if (LMS_Init(device)!=0)
            error();

        if (LMS_EnableChannel(device,LMS_CH_TX,0,true)!=0) 
            error();
        if (LMS_EnableChannel(device,LMS_CH_TX,1,true)!=0) 
            error();

        // preferred oversampling in RF 4x?? -> ERROR: SetFrequencyCGEN(3.2 MHz) - cannot deliver requested frequency
        // 0: auto
        if (LMS_SetSampleRate(device,sampleRate,0)!=0)
            error();

        if (LMS_SetLOFrequency(device,LMS_CH_TX, 0, frequency)!=0)
            error();
        if (LMS_SetLOFrequency(device,LMS_CH_TX, 1, frequency)!=0)
            error();
    
        if (LMS_SetAntenna(device, LMS_CH_TX, 0, LMS_PATH_TX1)!=0)   //TX1_1        
            error();
        if (LMS_SetAntenna(device, LMS_CH_TX, 1, LMS_PATH_TX1)!=0)   //TX2_1        
            error();

        // should be same amplitude -> 0.6(gnuradio rx_gain=0)
        // channel_0 amplitude tends to be more lerger than channel_1
        // 
        if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 0.39) != 0)
            error();
        if (LMS_SetNormalizedGain(device, LMS_CH_TX, 1, 0.68) != 0)
            error();

        // 00090726074F2C27
        // if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 0.68) != 0)
        //     error();
        // if (LMS_SetNormalizedGain(device, LMS_CH_TX, 1, 0.68) != 0)
        //     error();


        uint8_t gpio_val = 0;
        if (LMS_GPIORead(device, &gpio_val, 1)!=0) //1 byte buffer is enough to read 8 GPIO pins on LimeSDR-USB
            error();

        uint8_t gpio_dir = 0x00; //set bits 0,1,2,3 and 7
        if (LMS_GPIODirWrite(device, &gpio_dir, 1)!=0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
            error();
        std::cout << "Read GPIO:" << std::endl;
        if (LMS_GPIORead(device, &gpio_val, 1)!=0)
            error();
        print_gpio(gpio_val);

        bool set = false;
        std::cout << "GPIO_1 state: " << set << std::endl;

        runningTx1 = true;
        runningTx2 = true;
        // runningRx = true;
        std::thread threadTx1 = std::thread(StreamTx1);
        std::thread threadTx2 = std::thread(StreamTx2);

        this_thread::sleep_for(chrono::seconds(1));

        printf("waiting for GPIO_1 HIGH\n");
        while (!set) {
            uint8_t gpio_val = 0;
            if (LMS_GPIORead(device, &gpio_val, 1)!=0) //1 byte buffer is enough to read 8 GPIO pins on LimeSDR-USB
                error();
            set = gpio_val&(1<<1);
            start = gpio_val&(1<<1);
        }
        
        printf("set true!!");
        this_thread::sleep_for(chrono::seconds(streamTime));
        runningTx1 = false;
        threadTx1.join();
        runningTx2 = false;
        threadTx2.join();
        printf("joined\n");


        if (LMS_Close(device)==0)
            cout << "Closed" << endl;
    }
    return 0;
}