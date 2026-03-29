#include <stdio.h>
#include <stdlib.h> 
#include <strings.h>
#include <math.h> 
#include "lodepng.h" 

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

    size = width * height * 4;
    bw_size = width * height;
    
    
    unsigned char* bw_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    unsigned char* blr_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    unsigned char* finish = (unsigned char*)malloc(size*sizeof(unsigned char)); 
 
    // Например, поиграли с  контрастом
    contrast(bw_pic, bw_size); 
        // посмотрим на промежуточные картинки
    write_png("contrast.png", finish, width, height);
    
    // поиграли с Гауссом
    Gauss_blur(bw_pic, blr_pic, width, height); 
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
