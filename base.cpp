// ============================================
// Bibliotecas para ler e escrever PNG (header-only)
// ============================================
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// ============================================
// Bibliotecas padrão
// ============================================
#include <iostream>
#include <cmath>
#include <chrono>  // medição de tempo

using namespace std;
using namespace std::chrono;

int main() {

    // ============================================
    // Variáveis da imagem
    // ============================================
    int width, height, channels;

    // ============================================
    // Medição total
    // ============================================
    auto t_total_start = high_resolution_clock::now();

    // ============================================
    // 1. Leitura da imagem (PNG → memória linear)
    // ============================================
    auto t0 = high_resolution_clock::now();

    // Forçamos 3 canais (RGB) → evita problemas com PNG RGBA
    unsigned char* input = stbi_load("arara.png", &width, &height, &channels, 3);

    if (!input) {
        cout << "Erro ao carregar imagem!" << endl;
        return -1;
    }

    auto t1 = high_resolution_clock::now();

    // ============================================
    // Alocação de memória (buffers lineares)
    // ============================================
    // Cada pixel grayscale ocupa 1 byte
    unsigned char* gray   = new unsigned char[width * height];
    unsigned char* output = new unsigned char[width * height];

    // ============================================
    // 2. RGB -> GRAYSCALE
    // ============================================
    // Cada pixel RGB (3 canais) vira 1 valor (intensidade)
    // Esse padrão é ideal para CUDA: acesso linear e previsível
    // ============================================

    auto t2 = high_resolution_clock::now();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            // Índice linear do pixel (posição no vetor)
            int idx = (y * width + x) * 3;

            // Acesso aos canais (layout intercalado: RGBRGB...)
            unsigned char r = input[idx];
            unsigned char g = input[idx + 1];
            unsigned char b = input[idx + 2];

            // Conversão para escala de cinza (luminância)
            gray[y * width + x] = 0.299f * r + 0.587f * g + 0.114f * b;
        }
    }

    auto t3 = high_resolution_clock::now();

    // ============================================
    // 3. SOBEL (detecção de bordas)
    // ============================================
    // Cada pixel depende de uma vizinhança 3x3
    // Esse padrão vira um kernel CUDA com threads 2D
    // ============================================

    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    auto t4 = high_resolution_clock::now();

    // Ignoramos bordas → simplifica paralelização
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {

            int sumX = 0;
            int sumY = 0;

            // Janela 3x3 (convolução)
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {

                    int pixel = gray[(y + ky) * width + (x + kx)];

                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }

            // Magnitude do gradiente
            int magnitude = sqrt(sumX * sumX + sumY * sumY);

            // Saturação (limite de 8 bits)
            if (magnitude > 255) magnitude = 255;

            output[y * width + x] = (unsigned char)magnitude;
        }
    }

    auto t5 = high_resolution_clock::now();

    // ============================================
    // 4. Escrita da imagem
    // ============================================
    auto t6 = high_resolution_clock::now();

    // 1 canal → grayscale
    stbi_write_png("saida.png", width, height, 1, output, width);

    auto t7 = high_resolution_clock::now();

    auto t_total_end = high_resolution_clock::now();

    // ============================================
    // Cálculo dos tempos (em milissegundos)
    // ============================================
    auto t_load   = duration_cast<milliseconds>(t1 - t0).count();
    auto t_gray   = duration_cast<milliseconds>(t3 - t2).count();
    auto t_sobel  = duration_cast<milliseconds>(t5 - t4).count();
    auto t_write  = duration_cast<milliseconds>(t7 - t6).count();
    auto t_total  = duration_cast<milliseconds>(t_total_end - t_total_start).count();

    // ============================================
    // Saída de desempenho
    // ============================================
    cout << "=====================================\n";
    cout << "        Relatorio de Tempo\n";
    cout << "=====================================\n";
    cout << "Leitura (PNG):     " << t_load  << " ms\n";
    cout << "Grayscale:         " << t_gray  << " ms\n";
    cout << "Sobel:             " << t_sobel << " ms\n";
    cout << "Escrita (PNG):     " << t_write << " ms\n";
    cout << "-------------------------------------\n";
    cout << "Tempo total:       " << t_total << " ms\n";
    cout << "=====================================\n";

    cout << "Bordas detectadas!" << endl;

    // ============================================
    // Liberação de memória
    // ============================================
    stbi_image_free(input);
    delete[] gray;
    delete[] output;

    return 0;
}
