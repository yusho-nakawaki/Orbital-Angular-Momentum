/*

    2022/11/26
    - add GPIO pin working
    - GPIO_1 pin is trigger for start communication
    - should connect Vin, GND, Pin#1 and something like arduino
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
const double sampleRate = 1e6;
const int streamTime = 10;
const int timeout_ms = 10000;
bool runningTx;
bool runningRx;
bool start = false;

int error() {
  cout << "ERROR: " << LMS_GetLastErrorMessage() << endl;
  LMS_Close(device);
  exit(-1);
}

void StreamTx() {
    std::ifstream in_stream_re;
    in_stream_re.open("input_re_include_zero.dat");
    std::ifstream in_stream_im;
    in_stream_im.open("input_im_include_zero.dat"); 
    if(in_stream_re.fail() || in_stream_im.fail()){	
		std::cerr << "can't open file\n";
		exit(1);
	}

    int array_size = 1024*16;
    int linenum = 0;
    char* buf_re = new char[array_size];
    char* buf_im = new char[array_size];
    while(in_stream_re.getline(buf_re, sizeof(buf_re))){
		linenum++;
	}
    std::cerr << "number line of datFile = " << linenum << "\n";

    in_stream_re.clear();
	in_stream_re.seekg(0, std::ios::beg); // return top line of the file

    array_size = linenum;
    int tx_size = linenum;
    float* tx_buffer = new float[2*tx_size];

	for(int i=0 ; i<tx_size ; i++){
		in_stream_re.getline(buf_re,sizeof(buf_re));
		in_stream_im.getline(buf_im,sizeof(buf_im));
		tx_buffer[2*i] = atoi(buf_re);
		tx_buffer[2*i+1] = atoi(buf_im);
		// tx_buffer[2*i] = 1;
		// tx_buffer[2*i+1] = 0;
        // printf("%.1f %+.1fi\n", tx_buffer[2*i], tx_buffer[2*i+1]);
	}

    in_stream_re.close();
    delete[] buf_re;
    delete[] buf_im;

    lms_stream_t tx_stream;
    tx_stream.channel = 0;
    tx_stream.fifoSize = 16*1024;
    tx_stream.throughputVsLatency = 0.5;
    tx_stream.dataFmt = lms_stream_t::LMS_FMT_F32;
    tx_stream.isTx = true;
    lms_stream_meta_t meta_tx;   
    meta_tx.waitForTimestamp = false;
    meta_tx.flushPartialPacket = false;
    meta_tx.timestamp = 0;
    if (LMS_SetupStream(device, &tx_stream) != 0)
        error();

    LMS_StartStream(&tx_stream);
    printf("start stream tx\n");

    while (runningTx) {
        int ret = LMS_SendStream(&tx_stream,tx_buffer,tx_size,&meta_tx,timeout_ms);
        if (ret != tx_size)
            cout << "error: samples sent: " << ret << "/" << tx_size << endl;;
    }   
    
    printf("finished TX\n");
    delete[] tx_buffer;

    LMS_StopStream(&tx_stream);
    LMS_DestroyStream(device,&tx_stream); 
}

void StreamRX() {
    const int sampleCnt = 1024*128;
    float* buffer = new float[sampleCnt*2];

    lms_stream_t rx_stream;
    rx_stream.channel = 0; 
    rx_stream.fifoSize = 16*1024;
    rx_stream.throughputVsLatency = 0.5;
    rx_stream.isTx = false;
    rx_stream.dataFmt = lms_stream_t::LMS_FMT_F32;
    lms_stream_meta_t meta_rx; 
    meta_rx.flushPartialPacket = false;
    meta_rx.waitForTimestamp = false;
    if (LMS_SetupStream(device, &rx_stream) != 0)
        error();

    LMS_StartStream(&rx_stream);

    FILE *data;
    const char *data_file = "out.dat";
    data = fopen(data_file,"w");

    while (runningRx) {
        int samplesRead = LMS_RecvStream(&rx_stream, buffer, sampleCnt, &meta_rx, timeout_ms);
        for (int j = 0; j < samplesRead; ++j) {
            // fprintf(data,"%+f%+fi\n", buffer[2*j], buffer[2*j+1]);
        }
    }
    fclose(data);
    delete[] buffer;
    LMS_StopStream(&rx_stream); //stream is stopped but can be started again with LMS_StartStream()
    LMS_DestroyStream(device, &rx_stream); //stream is deallocated and can no longer be used
}

void print_gpio(int gpio_val) {
    for (int i = 0; i < 8; i++) {
        bool set = gpio_val&(1<<i); 
        std::cout << "GPIO" << i <<": " << (set ? "High" : "Low") << std::endl;
    }
}

int main(int argc, char** argv) {
    int n= LMS_GetDeviceList(nullptr);
    if (LMS_Open(&device,NULL,NULL)!=0) //open first device
        error();

    if (LMS_Init(device)!=0)
        error();

    
    uint8_t gpio_val = 0x00;
    if (LMS_GPIORead(device, &gpio_val, 1)!=0)
        error();
    print_gpio(gpio_val);

    if (LMS_GPIOWrite(device, &gpio_val, 1)!=0) {
        error();
    }

    // /**
    //  * @param       dev     Device handle previously obtained by LMS_Open().
    //  * @param[in]   buffer  GPIO direction configuration(8 GPIO per byte, LSB first; 0 input, 1 output)
    //  * @param       len     number of bytes to write
    //  */
    uint8_t gpio_dir = 0xFF; //set bits 0,1,2,3 and 7
    if (LMS_GPIODirRead(device, &gpio_dir, 1)!=0)
        error();

    if (LMS_WriteFPGAReg(device, 0xC0, 0xFFFE)!=0) 
        error();
    

    // gpio_val = 0xFF;
    if (LMS_GPIORead(device, &gpio_val, 1)!=0)
        error();
    print_gpio(gpio_val);

    if (LMS_EnableChannel(device,LMS_CH_TX,0,true)!=0) 
        error();
    if (LMS_EnableChannel(device,LMS_CH_RX,0,true) != 0)
        error();

    // preferred oversampling in RF 4x?? -> ERROR: SetFrequencyCGEN(3.2 MHz) - cannot deliver requested frequency
    // 0: auto
    if (LMS_SetSampleRate(device,sampleRate,0)!=0)
        error();

    if (LMS_SetLOFrequency(device,LMS_CH_TX, 0, frequency)!=0)
        error();
    if (LMS_SetLOFrequency(device,LMS_CH_RX, 0, frequency) != 0)
        error();

    if (LMS_SetAntenna(device, LMS_CH_TX, 0, LMS_PATH_TX1)!=0)   //TX1_1        
        error();
    // LMS_PATH_LNAW, LMS_PATH_LNAH, LMS_PATH_LNAL, LMS_PATH_AUTO
    if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAL)!=0)   //RX1_1        
        error();

    if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 0.4) != 0)
        error();
    if (LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 0.4) != 0)
        error();

    
    // uint8_t gpio_val = 0xFF;
    // uint8_t gpio_dir = 0x8F; //set bits 0,1,2,3 and 7
    // uint8_t gpio_dir = 0x00; //set bits 0,1,2,3 and 7
    if (LMS_GPIODirWrite(device, &gpio_dir, 1)!=0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
        error();
    std::cout << "Read GPIO:" << std::endl;
    if (LMS_GPIORead(device, &gpio_val, 1)!=0)
        error();
    print_gpio(gpio_val);

    bool set = gpio_val&(1<<1);
    std::cout << "GPIO_1 state: " << set << std::endl;

    // @param address   Register address
    // @param val       Value to write
    if (LMS_WriteFPGAReg(device, 0xC0, 0xFFFE)!=0) 
        throw std::runtime_error ("Unable to set GPIO behavior.");
    if (LMS_GPIORead(device, &gpio_val, 1)!=0)
        error();
    print_gpio(gpio_val);

    runningTx = true;
    runningRx = true;
    std::thread threadTx = std::thread(StreamTx);
    std::thread threadRx = std::thread(StreamRX);
    this_thread::sleep_for(chrono::seconds(streamTime));
    runningRx = false;
    threadRx.join();
    runningTx = false;
    threadTx.join();
    printf("joined\n");


    if (LMS_Close(device)==0)
        cout << "Closed" << endl;
    return 0;
}