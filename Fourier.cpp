//Dominant note per time window, using a hand-written FFT (no DSP libraries).
//Usage: ./fourier file.wav [window_size]   (power of two, default 4096)
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
using namespace std;

const double PI = acos(-1.0);
const double FRECUENCIA_MINIMA = 27.5; //ignores DC and rumble below A0
const double UMBRAL_SILENCIO = 0.002; //RMS under this (~ -54 dBFS) is silence
const char* NOMBRES_NOTAS[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

struct Chunk{
    char id[4]; //"RIFF", "fmt ", "data", "LIST"...
    unsigned int size;
};

struct Format{ //content of the "fmt " chunk
    unsigned short format_type; //1 means PCM
    unsigned short channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
};

bool leerWav(const string& archivo, vector<float>& mono, unsigned int& sample_rate);
void fft(vector<double>& re, vector<double>& im, const vector<double>& tablaCos, const vector<double>& tablaSen);

int main(int argc, char** argv){
    if (argc < 2){
        cerr << "Usage: " << argv[0] << " file.wav [window_size=4096]" << endl;
        return 1;
    }
    int N = argc > 2 ? atoi(argv[2]) : 4096;
    if (N < 16 || (N & (N - 1))){
        cerr << "window_size must be a power of two (>= 16)" << endl;
        return 1;
    }
    int hop = N / 2; //50% overlap

    vector<float> samples;
    unsigned int sample_rate;
    if (!leerWav(argv[1], samples, sample_rate)) return 1;
    int num_samples = samples.size();
    if (num_samples < N){
        cerr << "Audio is shorter than one window (" << N << " samples)" << endl;
        return 1;
    }

    vector<double> window(N), tablaCos(N / 2), tablaSen(N / 2), re(N), im(N), power(N / 2 + 1);
    for (int i = 0; i < N; i++) window[i] = 0.5 - 0.5 * cos(2 * PI * i / N); //Hann
    for (int k = 0; k < N / 2; k++){
        tablaCos[k] = cos(2 * PI * k / N);
        tablaSen[k] = -sin(2 * PI * k / N);
    }
    int binMinimo = max(1, (int)ceil(FRECUENCIA_MINIMA * N / sample_rate));

    cout << fixed << "# " << sample_rate << " Hz | window " << N << " samples (" << setprecision(1) << 1000.0 * N / sample_rate
         << " ms) | bin width " << setprecision(2) << (double)sample_rate / N << " Hz | hop " << hop << '\n';
    cout << "# start(s)   freq(Hz)  note\n";

    for (int inicio = 0; inicio + N <= num_samples; inicio += hop){
        double tiempo = (double)inicio / sample_rate, energia = 0;
        for (int i = 0; i < N; i++){
            re[i] = samples[inicio + i] * window[i];
            im[i] = 0;
            energia += (double)samples[inicio + i] * samples[inicio + i];
        }
        if (sqrt(energia / N) < UMBRAL_SILENCIO){
            cout << setprecision(3) << setw(10) << tiempo << "   -          silence\n";
            continue;
        }

        fft(re, im, tablaCos, tablaSen);

        for (int k = 0; k <= N / 2; k++) power[k] = re[k] * re[k] + im[k] * im[k]; //real input: only half is useful
        int peak = binMinimo;
        for (int k = binMinimo; k < N / 2; k++)
            if (power[k] > power[peak]) peak = k;

        //a bin is wide, so we fit a parabola (log scale) through the peak and its neighbours
        double izq = log(power[peak - 1] + 1e-30), centro = log(power[peak] + 1e-30), der = log(power[peak + 1] + 1e-30);
        double denom = izq - 2 * centro + der;
        double delta = denom != 0 ? 0.5 * (izq - der) / denom : 0;
        delta = max(-0.5, min(0.5, delta));
        double frecuencia = (peak + delta) * sample_rate / N;

        double midi = 69 + 12 * log2(frecuencia / 440.0); //A4 = 440 Hz = 69
        int notaMidi = lround(midi);
        int cents = lround((midi - notaMidi) * 100);
        cout << setprecision(3) << setw(10) << tiempo << "   " << setprecision(2) << setw(8) << frecuencia << "   "
             << NOMBRES_NOTAS[notaMidi % 12] << notaMidi / 12 - 1 << "  (" << showpos << cents << noshowpos << " cents)\n";
    }
    return 0;
}

bool leerWav(const string& archivo, vector<float>& mono, unsigned int& sample_rate){
    ifstream file(archivo, ios::binary);
    if (!file){
        cerr << "File cannot be open!!" << endl;
        return false;
    }
    file.seekg(0, ios::end);
    long long tamArchivo = file.tellg();
    file.seekg(0);

    Chunk riff;
    char wave[4]; //always "WAVE"
    file.read(reinterpret_cast<char*>(&riff), sizeof(Chunk));
    file.read(wave, 4);
    if (!file || string(riff.id, 4) != "RIFF" || string(wave, 4) != "WAVE"){
        cerr << "Not a WAV file" << endl;
        return false;
    }

    //a wav is a list of chunks (id + size + content)
    Format fmt = {};
    Chunk chunk;
    while (file.read(reinterpret_cast<char*>(&chunk), sizeof(Chunk))){
        string id(chunk.id, 4);
        int relleno = chunk.size % 2; //chunks padded even
        if (id == "fmt "){
            if (chunk.size < sizeof(Format)){
                cerr << "Broken fmt chunk" << endl;
                return false;
            }
            file.read(reinterpret_cast<char*>(&fmt), sizeof(Format));
            file.seekg(chunk.size - sizeof(Format) + relleno, ios::cur); //fmt can be longer than Format
        }else if (id == "data"){
            if (fmt.format_type != 1 || fmt.bits_per_sample != 16 || fmt.channels == 0 || fmt.sample_rate == 0){
                cerr << "Unsupported WAV format. Use 16-bit PCM (ffmpeg -i input.mp3 output.wav)" << endl;
                return false;
            }
            long long quedan = tamArchivo - (long long)file.tellg();
            long long bytes = min<long long>(chunk.size, quedan); //streamed or cut files can claim more than they have
            int num_samples = bytes / 2; //all channels together
            vector<short> datos(num_samples);
            file.read(reinterpret_cast<char*>(datos.data()), num_samples * sizeof(short));

            sample_rate = fmt.sample_rate;
            int frames = num_samples / fmt.channels;
            mono.resize(frames);
            for (int i = 0; i < frames; i++){
                int suma = 0;
                for (int c = 0; c < fmt.channels; c++) suma += datos[i * fmt.channels + c];
                mono[i] = suma / (32768.0f * fmt.channels); //mix down to mono in [-1, 1]
            }
            return true;
        }else{
            file.seekg(chunk.size + relleno, ios::cur);
        }
    }
    cerr << "No data chunk found" << endl;
    return false;
}

//iterative radix-2 Cooley-Tukey, in place. tablaCos[k] = cos(2*pi*k/n), tablaSen[k] = -sin(2*pi*k/n)
void fft(vector<double>& re, vector<double>& im, const vector<double>& tablaCos, const vector<double>& tablaSen){
    int n = re.size();

    for (int i = 1, j = 0; i < n; i++){ //bit-reversal reordering
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j){
            swap(re[i], re[j]);
            swap(im[i], im[j]);
        }
    }

    for (int tam = 2; tam <= n; tam <<= 1){ //butterflies of size 2, 4, 8... n
        int mitad = tam / 2, paso = n / tam;
        for (int i = 0; i < n; i += tam){
            for (int k = 0; k < mitad; k++){
                double wr = tablaCos[k * paso], wi = tablaSen[k * paso];
                int a = i + k, b = a + mitad;
                double tr = re[b] * wr - im[b] * wi;
                double ti = re[b] * wi + im[b] * wr;
                re[b] = re[a] - tr;
                im[b] = im[a] - ti;
                re[a] += tr;
                im[a] += ti;
            }
        }
    }
}
