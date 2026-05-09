#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h> 
#include "lodepng.h"



//                             --- Функции загрузки и записи ---


// принимаем на вход: имя файла, указатели на int для хранения прочитанной ширины и высоты картинки
// возвращаем указатель на выделенную память для хранения картинки
// Если память выделить не смогли, отдаем нулевой указатель и пишем сообщение об ошибке
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height) {
    unsigned char* image = NULL; 
    int error = lodepng_decode32_file(&image, width, height, filename);
    if(error != 0) {
        printf("error %u: %s\n", error, lodepng_error_text(error)); 
    }
    return (image);
}

// принимаем на вход: имя файла для записи, указатель на массив пикселей,  ширину и высоту картинки
// Если преобразовать массив в картинку или сохранить не смогли,  пишем сообщение об ошибке
// RGBA - это 4 канала
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height){
  unsigned char* png;
  size_t pngsize;
  int error = lodepng_encode32(&png, &pngsize, image, width, height);
  if(error == 0) {
      lodepng_save_file(png, pngsize, filename);
    } 
  else{ 
    printf("error %u: %s\n", error, lodepng_error_text(error));
    }
  free(png);
}

//так как снимок в ч/б 
// Записываем в 1 канал
void write_png_gray(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
    unsigned char* png;
    size_t pngsize;
    // Используем LCT_GREY для записи
    int error = lodepng_encode_memory(&png, &pngsize, image, width, height, LCT_GREY, 8);
    if (error == 0) {
        lodepng_save_file(png, pngsize, filename);
    } 
    else {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    free(png);
}



//                                  --- Обработка изображения ---
// создадим рамки - рабочую область, чтобы не мучиться с сущей и надписями
typedef struct WorkArea{
    int x, y;      // лев_верх
    int w;      // ширина области
    int h;      // высота области
} WorkArea;

// 1. Рекурсивно ищемб возвращаем площадь одного обьекта
int get_object_size(unsigned char *pic, int x, int y, int img_width, WorkArea area) {
    if (x < area.x || x >= area.x + area.w || y < area.y || y >= area.y + area.h)
        return 0;

    int idx = y * img_width + x;
    if (pic[idx] != 255) // Ищем только "проявленные" пиксели
        return 0;

    pic[idx] = 254; // gомечаем как посещенный

    return 1 + get_object_size(pic, x + 1, y, img_width, area)+ get_object_size(pic, x - 1, y, img_width, area)+ get_object_size(pic, x, y + 1, img_width, area)+ get_object_size(pic, x, y - 1, img_width, area);
}


// 2. Гауссово размыттие только внутри области
void Gauss_blur_area(unsigned char *col, unsigned char *blr_pic, int width, WorkArea area){ 
    for(int i = area.y + 1; i < area.y + area.h - 1; i++) {
        for(int j = area.x + 1; j < area.x + area.w - 1; j++) {
            int idx = width * i + j; //центральный пиксель
            float sum = 0.084 * col[idx] + 
                        0.084 * col[width * (i + 1) + j] + 0.084 * col[width * (i - 1) + j] +
                        0.084 * col[width * i + (j + 1)] + 0.084 * col[width * i + (j - 1)] +
                        0.063 * col[width * (i + 1) + (j + 1)] + 0.063 * col[width * (i + 1) + (j - 1)] +
                        0.063 * col[width * (i - 1) + (j + 1)] + 0.063 * col[width * (i - 1) + (j - 1)];
            blr_pic[idx] = (unsigned char)sum;
        }
    } 
} 

// 3. Делаем корабль чисто белым, море-черным
//Хотим найти границу цвета между морем и кораблями
// считаем среднюю яркость в зоне и брем, что в 2 раза ярче неё
void contrast_area(unsigned char *col, int img_width, WorkArea area, int threshold) { 
    for(int i = area.y; i < area.y + area.h; i++) {
        for(int j = area.x; j < area.x + area.w; j++) {
            int idx = i * img_width + j;
            // Теперь решаем, что считать белым
            if(col[idx] >= threshold){
                col[idx] = 255;
            }   
            else 
                col[idx] = 0;              
        }
    }
} 

// 4. Ищем и подсчитываем танкеры - белые пиксели
int count_ships(unsigned char *pic, int img_width, WorkArea area) {
    int ships_found = 0;
    for (int i = area.y; i < area.y + area.h; i++) {
        for (int j = area.x; j < area.x + area.w; j++) {
            if (pic[i * img_width + j] == 255) {
                // если нашлт белую точку-узнаем размер обьекта
                int size = get_object_size(pic, j, i, img_width, area);
                // если это кораблт (от4 до 20 пиксл)
                if (size >= 4 && size < 20) {
                    ships_found++;
                }
            }
        }
    }
    return ships_found;
}

// 5. Общий процесс
void process_area(WorkArea area, unsigned char *bw_pic, unsigned char *blr_pic, int img_width, int threshold) {
    printf("Зона [%d,%d]: ", area.x, area.y);
    
    // Копируем кусок картинк для обработки
    for (int i = area.y; i < area.y + area.h; i++) {
        memcpy(blr_pic + i * img_width + area.x, bw_pic + i * img_width + area.x, area.w);
    }
    
    Gauss_blur_area(bw_pic, blr_pic, img_width, area);
    contrast_area(blr_pic, img_width, area, threshold);
    int total_ships = count_ships(blr_pic, img_width, area);
    printf("Нашли %d\n", total_ships);
}


// Перевод RGBA в Gray
void rgba_to_gray(unsigned char* rgba, unsigned char* gray, int w, int h) {
    for (int i = 0; i < w * h; i++) {
        gray[i] = rgba[i * 4];
    }
}


//                                        --- Функция main ---

int main() { 
    const char* filename = "highlighted_areas.png"; 
    unsigned int width, height;
    
    // Прочитали картинку
    unsigned char* picture = load_png(filename, &width, &height); 
    if (picture == NULL){ 
        printf("Problem reading picture from the file %s. Error.\n", filename); 
        return -1; 
    } 

    printf("Image loaded: %ux%u\n", width, height);

    int bw_size = width * height;
    unsigned char* bw_pic = (unsigned char*)malloc(bw_size); 
    unsigned char* blr_pic = (unsigned char*)malloc(bw_size); 
    
    memset(blr_pic, 0, bw_size);
        
    // Перевод в Gray (берем канал R)
    rgba_to_gray(picture, bw_pic, width, height);

    WorkArea zones[] = {
        {632, 34, 91, 210}, {524, 160, 82, 119}, {637, 272, 61, 38},
        {550, 320, 63, 22}, {500, 326, 50, 13},  {514, 337, 276, 78},
        {524, 444, 52, 89}, {533, 533, 41, 25},  {1071, 403, 26, 32},
        {990, 586, 104, 50}, {886, 620, 45, 27}, {498, 299, 70, 17},
        {573, 420, 375, 180}
    };

    int num_zones = 13;
    int threshold = 80;  //снизим порог вместо 180
    int total_ships = 0;

    //чистим чтобы вокгуг зон все оставалось черным
    memset(blr_pic, 0, width * height); 

    for (int i = 0; i < num_zones; i++) {

        // копируеми исхолные пиксели зоны в буфер
        for (int y = zones[i].y; y < zones[i].y + zones[i].h; y++) {
            memcpy(blr_pic + y * width + zones[i].x, bw_pic + y * width + zones[i].x, zones[i].w);
        }
        // Первое, приводим к общему значению(блюром)
        Gauss_blur_area(bw_pic, blr_pic, width, zones[i]);
        
        
        //превращаем сереое в белое 
        contrast_area(blr_pic, width, zones[i], threshold);
        
        
        //считаем кораблем >= 4 пикселя
        int ships_in_zone = 0;
        for (int y = zones[i].y; y < zones[i].y + zones[i].h; y++) {
            for (int x = zones[i].x; x < zones[i].x + zones[i].w; x++) {
                if (blr_pic[y * width + x] == 255) {
                    int obj_size = get_object_size(blr_pic, x, y, width, zones[i]);
                    if (obj_size >= 4) {
                        ships_in_zone++;
                    }
                }
            }
        }
        
        printf("Зона %d: Найдено: %d \n", i + 1, ships_in_zone);
        total_ships += ships_in_zone;
    }

    write_png_gray("after_blur.png", blr_pic, width, height); // Сохраняем результат блюра
    write_png_gray("after_contrast.png", blr_pic, width, height); // Видим только белые пятна

    // Сохраняем финальную маску (теперь тут только крупные объекты)
    write_png_gray("final_clusters.png", blr_pic, width, height); 
    printf("Итоговое количество танкеров: %d\n", total_ships);

    
    // не забыли почистить память!
    free(bw_pic); 
    free(blr_pic); 
    free(picture); 
    
    return 0; 
}
