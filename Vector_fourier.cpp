#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <cstdlib>


using namespace std;

const double E = 2.71828182845904523536;


struct WAV_Header{
    char riff[4]; ////Always RIFF if it's a .wav audio, we can use this to compare and let the file pass as media.
    unsigned int overall_size;
    char wave[4]; //Always WAVE if it's a .wav audio, we can use this to compare and let the file pass as audio.
    char fmt_chunk_marker[4]; //Always fmt, convention flag
    unsigned int length_fmt; //Lenght of format listed above
    unsigned short format_type; // 2bytes format description. 1 means PCM, i know there exist LPCM but don't know how to implement it at least in this way.
    unsigned short channels; //2byte, number of chanels making it mono or stereo
    unsigned int sample_rate; //4bytes always, common values are 44100 Hz used on CD.
    unsigned int byte_rate; //4bytes, (Sample Rate * BitsPerSample * Channels) / 8.
    unsigned short block_align; //2 byte, Indicates the size in bytes of a unit of sample data. 
    unsigned short bits_per_sample;
    char data_chunk_header[4]; //4 characters flag, "data"
    unsigned int data_size; //size of the next data, the sample data.
};

struct WAV_Sample{
    long double time;
    short amplitude;
};

void read(string& file_name, int& number, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample, bool& file_read);
int menu(string& file_name, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample, vector<WAV_Sample>& sample_trans);
void display(vector<WAV_Sample> sample, int num_samples, int number);
void semi_fourier(vector<WAV_Sample> sample, vector<WAV_Sample>& sample_trans, int num_samples);
void var_to_datafile(vector<WAV_Sample> sample, vector<WAV_Sample> sample_trans, int num_samples, int number);



int main(){
    vector<WAV_Sample> sample;
    vector<WAV_Sample> sample_trans;
    string file_name;
    WAV_Header header;
    int num_samples;

    menu(file_name, header, num_samples, sample, sample_trans);

}

int menu(string& file_name, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample, vector<WAV_Sample>& sample_trans){
    int number = 1;
    bool file_read = false;

    while (number != 0){
    cout << endl << "MENU:" << endl;
    cout << "0. Exit." << endl;
    cout << "1. Insert the .wav file." << endl;
    cout << "2. Display." << endl; // for the moment a numerical display, (J4iva).
    cout << "3. Display transformed." << endl; // for the moment a numerical display, (J4iva).
    //...
    cout << "Enter the option number: ";
    cin >> number;

    switch (number)
    {
    case 0:
        cout << "Exiting the program..." << endl;
        break;
    case 1:
        read(file_name, number, header, num_samples, sample, file_read);
        break;
    
    case 2:
        
        if (file_read == false){
            cout << endl << "You must upload a .wav file first." << endl;
        } else{
        var_to_datafile(sample, sample_trans, num_samples, number);
        display(sample, num_samples, number);
        }
        break;

    case 3:
    
    if (file_read == false){
        cout << endl << "You must upload a .wav file first." << endl;
        }else{
        var_to_datafile(sample, sample_trans, num_samples, number);
        display(sample_trans, num_samples, number);
        } 
        break;

    case 404:
        cout << endl << "An error ocurred, going back to the menu..." << endl;
        break;
    }
  }

  return 0; //For the moment, (J4iva).

}



void read(string& file_name, int& number, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample, bool& file_read){
    vector<short> temp_sample; //Used to read the samples directly, this could cause an error so keep track, (J4iva).
    file_read = true;

    cout <<  endl << "Insert the file location here: ";
    cin.ignore();
    getline(cin, file_name); //Name of the file for me /home/jaiva/Audio_filter/test-3s-mono.wav , (J4iva).

    ifstream file(file_name, ios::binary);

    if (!file){
        cout << endl << "File cannot be open!" << endl;
        number = 404;
        file_read = false;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(WAV_Header)); // Making the first part of the binary fit the strcut, (J4iva).

    if (header.bits_per_sample != 16 || header.channels != 1 || header.format_type != 1){
        cout << endl << "Unsupported WAV file format or not WAV file. Use .wav 16-bit mono PCM" << endl;
        number = 404;
        file_read = false;
    } //This could be changed to variables comparation if we create a personalize file format of the .wav file, (J4iva).


    //Second part of the read, sample part, (J4iva).
     num_samples = header.data_size / (header.bits_per_sample / 8);

     temp_sample.resize(num_samples); //making the vector equal to the number of samples so fit the next sample, (J4iva).
     file.read(reinterpret_cast<char*>(temp_sample.data()), num_samples * sizeof(short));
     file.close();

    for (int i = 0; i < num_samples; i++){
        WAV_Sample current_sample;
        current_sample.amplitude = temp_sample[i];
        current_sample.time = (static_cast<long double>(i) * 1000.0) / header.sample_rate; // This will make the vector struct bijective, (J4iva).
        sample.push_back(current_sample);
    }

    if (file_read == true){
        cout << "File reading complete succesfully, going back to the menu... " << endl;
    }
    
}

void display(vector<WAV_Sample> sample, int num_samples, int number){
    
    cout << number << endl;

    if (number == 2){
        ofstream plotfile("original_plot.gp"); //This part was taken from stack overflow and modified, (J4iva).
        plotfile << "set title 'Audio Samples' \n";
        plotfile << "set xlabel 'Time (ms)'\n";
        plotfile << "set ylabel 'Amplitude'\n";
        plotfile << "plot 'wav_graph.dat' with lines title 'Audio Signal'\n";
        plotfile << "pause -1 'Press any key to exit...'\n";
        plotfile.close();
        system("gnuplot original_plot.gp");
    }
    
    if(number == 3){
        ofstream plotfile("transform_plot.gp"); //This part was taken from stack overflow and modified, (J4iva).
        plotfile << "set title 'Semi-Fourier Transform' \n";
        plotfile << "set xlabel 'Frequency'\n";
        plotfile << "set ylabel 'Magnitude'\n";
        plotfile << "plot 'wav_graph_transformed.dat' with lines title 'Semi-Fourier Transform'\n";
        plotfile << "pause -1 'Press any key to exit...'\n";
        plotfile.close();
        system("gnuplot transform_plot.gp");
    }


}

void semi_fourier(vector<WAV_Sample> sample, vector<WAV_Sample>& sample_trans, int num_samples){ //This function is producing a core dumped, must debug, (J4iva).


    for (int i = 0; i < num_samples; i++){
        WAV_Sample current_sample;
        complex<double> pow_complex (0, -2*M_PI*(1/10)*sample[i].time); // generating the complex number, real part is 0 in the fourier trans. M_PI is a long doube of pi, (J4iva).

        current_sample.amplitude = sample[i].amplitude * pow(E, imag(pow_complex)); // imag() refer to the imaginary part of the variable inside (part of complex library), sample_trans[i].amplitude = g(t_k)*e^(-2pi*f*t_k*i) , (J4iva).
        current_sample.time = sample[i].time;

        sample_trans.push_back(current_sample);
   
    }

    for (int i = 0; i < num_samples; i++){
        cout << i << "Time: " << sample_trans[i].time << "ms, Amplitude: " << sample_trans[i].amplitude << endl;
    }

}

void var_to_datafile(vector<WAV_Sample> sample, vector<WAV_Sample> sample_trans, int num_samples, int number){
    if (number == 2){
        ofstream datafile("wav_graph.dat");
        for (int i = 0; i < num_samples; i++){
            datafile << sample[i].time << " " << sample[i].amplitude << endl;
        }
    datafile.close();
    }else if(number == 3){
         ofstream datafile("wav_graph_transformed.dat");
        for (int i = 0; i < num_samples; i++){
            datafile << sample[i].time << " " << sample[i].amplitude << endl;
        }
    datafile.close();
    }

    


}