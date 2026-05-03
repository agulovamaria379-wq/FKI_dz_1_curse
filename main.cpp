#include <stdio.h>
#include <stdlib.h> 
#include <strings.h>
#include <math.h> 
#include "lodepng.h" 


//                             --- Функции загрузки и записи ---
//так как снимок в ч/б
void rgba_to_gray(unsigned char* rgba, unsigned char* gray, int w, int h) {
    for (int i = 0; i < w * h; i++) {
        gray[i] = rgba[i * 4]; //Берем красный канал как интенсивность
    }
// принимаем на вход: имя файла, указатели на int для хранения прочитанной ширины и высоты картинки
// возвращаем указатель на выделенную память для хранения картинки
// Если память выделить не смогли, отдаем нулевой указатель и пишем сообщение об ошибке
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height) 
{
  unsigned char* image = NULL; 
  int error = lodepng_decode32_file(&image, width, height, filename);
  if(error != 0) {
    printf("error %u: %s\n", error, lodepng_error_text(error)); 
  }
  return (image);
}

// принимаем на вход: имя файла для записи, указатель на массив пикселей,  ширину и высоту картинки
// Если преобразовать массив в картинку или сохранить не смогли,  пишем сообщение об ошибке
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  unsigned char* png;
  long unsigned int pngsize;
  int error = lodepng_encode32(&png, &pngsize, image, width, height);
  if(error == 0) {
      lodepng_save_file(png, pngsize, filename);
  } else { 
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


// подсчет танкеров - белых пикселей
int count_pixels(unsigned char *pic, int img_width, WorkArea area) {
    int count = 0;
    for (int i = area.y; i < area.y + area.h; i++) {
        for (int j = area.x; j < area.x + area.w; j++) {
            if (pic[i * img_width + j] == 255) {
                count++;
            }
        }
    }
    return count;
}


void process_area(WorkArea area, unsigned char *bw_pic, unsigned char *blr_pic, int img_width) {
    printf("x=%d, y=%d... ", area.x, area.y);

    Gauss_blur_area(bw_pic, blr_pic, img_width, area);
    contrast_area(blr_pic, img_width, area);
    int tankers = count_pixels(blr_pic, img_width, area);
    printf("%d\n", tankers);
}


// вариант огрубления серого цвета в ЧБ в нутри области
void contrast_area(unsigned char *col, int img_width, WorkArea area) { 
    for(int i = area.y; i < area.y + area.h; i++) {
        for(int j = area.x; j < area.x + area.w; j++) {
            int idx = i * img_width + j;
            if(col[idx] < 120){
                col[idx] = 0;}   // все что темнее 120 - в ноль
            else 
                col[idx] = 255;              //яркое - в белый
        }
    }
} 

// Гауссово размыттие только внутри области
void Gauss_blur_area(unsigned char *col, unsigned char *blr_pic, int width, WorkArea area) { 
    for(int i = area.y + 1; i < area.y + area.h - 1; i++) {
        for(int j = area.x + 1; j < area.x + area.w - 1; j++) {
            int idx = width * i + j; //центральный пиксель
            blr_pic[width*i+j] = 0.084*col[width*i+j] + 0.084*col[width*(i+1)+j] + 0.084*col[width*(i-1)+j]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.084*col[width*i+(j+1)] + 0.084*col[width*i+(j-1)]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i+1)+(j+1)] + 0.063*col[width*(i+1)+(j-1)]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i-1)+(j+1)] + 0.063*col[width*(i-1)+(j-1)]; 
        } 
   return; 
} 

//  Место для экспериментов
void color(unsigned char *blr_pic, unsigned char *res, int size)
{ 
  int i;
    for(i=1;i<size;i++) 
    { 
        res[i*4]=40+blr_pic[i]+0.35*blr_pic[i-1]; 
        res[i*4+1]=65+blr_pic[i]; 
        res[i*4+2]=170+blr_pic[i]; 
        res[i*4+3]=255; 
    } 
    return; 
} 
  
int main() 
{ 
    const char* filename = "skull.png"; 
    unsigned int width, height;
    int size;
    int bw_size;
    
    // Прочитали картинку
    unsigned char* picture = load_png("skull.png", &width, &height); 
    if (picture == NULL)
    { 
        printf("Problem reading picture from the file %s. Error.\n", filename); 
        return -1; 
    } 

    int total_ships = 0;

    size = width * height * 4;
    bw_size = width * height;
    
    
    unsigned char* bw_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    unsigned char* blr_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    unsigned char* finish = (unsigned char*)malloc(size*sizeof(unsigned char));
    
    // Перевод в Gray (берем канал R)
    for(int i=0; i<bw_size; i++){
        bw_pic[i] = picture[i*4];
    }

    //
    WorkArea zones[] = {
        {632, 34, 91, 210}, {524, 160, 82, 119}, {637, 272, 61, 38},
        {550, 320, 63, 22}, {500, 326, 50, 13},  {514, 337, 276, 78},
        {524, 444, 52, 89}, {533, 533, 41, 25},  {1071, 403, 26, 32},
        {990, 586, 104, 50}, {886, 620, 45, 27}, {498, 299, 70, 17},
        {573, 420, 375, 180}
    };
    int num_zones = 13;


    for (int i = 0; i < 12; i++) {
        // Первое, приводим к общему значению(блюром)
        Gauss_blur(bw_pic, blr_pic, width, zones[i]);
        //превращаем сереое в белое 
        contrast(bw_pic, bw_size);
        //считаем кораблем >= 4 пикселя
        int ships_in_zone = 0;
        for (int y = zones[i].y; y < zones[i].y + zones[i].h; y++) {
            for (int x = zones[i].x; x < zones[i].x + zones[i].w; x++) {
                if (blr_pic[y * width + x] == 255) {
                    int size = get_object_size(blr_pic, x, y, width, zones[i]);
                    if (size >= 4) {
                        ships_in_zone++;
                }
            }
        }
    }
    printf("Зона %d: Найдено %d \n", i + 1, ships_in_zone);
    total_ships += ships_in_zone;
}
printf("Итоговое количество танкеров: %d\n", total_ships);


        // посмотрим на промежуточные картинки
    write_png("contrast.png", finish, width, height);
    
    // поиграли с Гауссом
    
    // посмотрим на промежуточные картинки
    write_png("gauss.png", finish, width, height);
    
    // сделали еще что-нибудь
    // .....
    // ....
    // ....
    // ....
    // ....
    // ....
    // ....
    //
    
    write_png("intermediate_result.png", finish, width, height);
    color(blr_pic, finish, bw_size); 
    
    // выписали результат
    write_png("picture_out.png", finish, width, height); 
    
    // не забыли почистить память!
    free(bw_pic); 
    free(blr_pic); 
    free(finish); 
    free(picture); 
    
    return 0; 
}
