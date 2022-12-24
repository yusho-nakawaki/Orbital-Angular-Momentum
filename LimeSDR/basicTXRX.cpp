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
bool runningTx;
bool runningRx;

int error() {
  cout << "ERROR: " << LMS_GetLastErrorMessage() << endl;
  LMS_Close(device);
  exit(-1);
}

void StreamTx() {
    std::ifstream in_stream_re;
    in_stream_re.open("input_re.dat");
    std::ifstream in_stream_im;
    in_stream_im.open("input_im.dat"); 
    if(in_stream_re.fail() || in_stream_im.fail()){	
		std::cerr << "ファイルを開けません\n";
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
	in_stream_re.seekg(0, std::ios::beg);

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
    printf("tx_buffer size:");
    cout << sizeof(tx_buffer) / sizeof(int) << "\n";

    printf("start TX!!\n");
    while (runningTx) {
        int ret = LMS_SendStream(&tx_stream,tx_buffer,tx_size,&meta_tx,1000);
        // printf("SendStream: %d\n", ret);
        // printf("tx_buffer size:");
        // cout << sizeof(tx_buffer) << "\n";
        if (ret != tx_size)
            cout << "error: samples sent: " << ret << "/" << tx_size << endl;;
    }   
    
    printf("finished TX\n");
    delete[] tx_buffer;

    LMS_StopStream(&tx_stream);
    LMS_DestroyStream(device,&tx_stream); 
}

void StreamRx() {
    lms_stream_t rx_stream;
    rx_stream.channel = 0; //channel number
    rx_stream.fifoSize = 16*1024; //fifo size in samples
    rx_stream.throughputVsLatency = 0.5; //some middle ground
    rx_stream.isTx = false; //RX channel
    rx_stream.dataFmt = lms_stream_t::LMS_FMT_F32;
    if (LMS_SetupStream(device, &rx_stream) != 0)
        error();

    LMS_StartStream(&rx_stream);

    FILE *data_output;
    const char *data_file = "out.dat"; // does it need pointer*?
    data_output = fopen(data_file,"w");
    int rx_size = 128*1024*streamTime;
    float* rx_buffer = new float[rx_size];

    printf("start RX!!\n");

    while (runningRx) {
        int samplesRead = LMS_RecvStream(&rx_stream,rx_buffer,rx_size,NULL,1000);
        for (int j=0; j<samplesRead; ++j) { 
            if (j%1000 == 0) {
                printf("%+f %fi, sampleRead:%d\n", rx_buffer[2*j], rx_buffer[2*j+1], samplesRead);
            }  
            fprintf(data_output,"%+f%+fi\n", rx_buffer[2*j], rx_buffer[2*j+1]);
            // std::complex<float> z(rx_buffer[2 * j], rx_buffer[2 * j + 1]);
            // fprintf(data_output,"%f\n", std::abs(z));
            // printf("%f\n", std::abs(z));
        }
    }
    printf("finished RX\n");

    fclose(data_output);
    delete[] rx_buffer;

    LMS_StopStream(&rx_stream);
    LMS_DestroyStream(device, &rx_stream);
}

void StreamRX2() {
        const int sampleCnt = 1024*128;
        float* buffer = new float[sampleCnt*2];
        lms_stream_t rx_stream;
        rx_stream.channel = 0; 
        rx_stream.fifoSize = 16*1024;
        rx_stream.throughputVsLatency = 0.5;
        rx_stream.isTx = false;
        rx_stream.dataFmt = lms_stream_t::LMS_FMT_F32;
        lms_stream_meta_t meta_rx; //Use metadata for additional control over sample receive function behavior
        meta_rx.flushPartialPacket = false; //currently has no effect in RX
        meta_rx.waitForTimestamp = false; //currently has no effect in RX
        if (LMS_SetupStream(device, &rx_stream) != 0)
            error();

        LMS_StartStream(&rx_stream);

        FILE *data;
        const char *data_file = "out.dat";
        data = fopen(data_file,"w");
        while (runningRx) {
            int samplesRead = LMS_RecvStream(&rx_stream, buffer, sampleCnt, &meta_rx, 1000);
            for (int j = 0; j < samplesRead; ++j) {
                fprintf(data,"%+f%+fi\n", buffer[2*j], buffer[2*j+1]);
                // std::complex<float> z(buffer[2 * j], buffer[2 * j + 1]);
                // fprintf(data,"%f\n", std::abs(z));
            }
        }
        fclose(data);
        delete[] buffer;
        LMS_StopStream(&rx_stream); //stream is stopped but can be started again with LMS_StartStream()
        LMS_DestroyStream(device, &rx_stream); //stream is deallocated and can no longer be used
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

        if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 1) != 0)
            error();
        if (LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 0.4) != 0)
            error();

        runningTx = true;
        runningRx = true;
        std::thread threadTx = std::thread(StreamTx);
        std::thread threadRx = std::thread(StreamRX2);
        this_thread::sleep_for(chrono::seconds(streamTime));
        runningRx = false;
        threadRx.join();
        runningTx = false;
        threadTx.join();
        printf("joined\n");

        if (LMS_Close(device)==0)
            cout << "Closed" << endl;
    }
    return 0;
}