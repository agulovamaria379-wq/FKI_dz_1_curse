#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct Matrix{
    int rows;
    int cols;
    double **data;
}Matrix;

Matrix init_matrix(int rows, int cols){
    Matrix m;
    m.rows = rows;
    m.cols = cols;
    m.data = (double **)malloc(rows*sizeof(double*));
    for(int i = 0; i<rows; i++){
        m.data[i] = (double*)malloc(cols*sizeof(double));
    }
    return m;
}
void free_matrix(Matrix *m) {
    for (int i = 0; i < m->rows; i++){
        free(m->data[i]);
    }
    
    free(m->data);
}


void elementary_swap(Matrix *m, int r1, int r2, int *sign) {
    if (r1 == r2){
        return;}
    double *temp = m->data[r1];
    m->data[r1] = m->data[r2];
    m->data[r2] = temp;
    *sign *= -1;
}


void elementary_add(Matrix *m, int target_row, int sourse_row, double coef, int start_col) { // не менем определитель
    for(int j = start_col; j< m->cols; j++){
        double value = coef*m->data[sourse_row][j];
        m->data[target_row][j] -= value;
    }
}


int find_pivot(Matrix *m, int col){ // ищем наибольш числоБ чтобы поставить наверх
    int max_row = col;
    for (int i = col + 1; i < m->rows; i++){
        if (fabs(m->data[i][col]) > fabs(m->data[max_row][col])){
            max_row = i;
        }
    }
    return max_row;

}


double calculate_determinant(Matrix m) {
    if (m.rows != m.cols){
        return 0.0;
    }

    int n = m.rows;
    double det = 1.0;
    int sign = 1;

    Matrix tmp = init_matrix(n, n);
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            tmp.data[i][j] = m.data[i][j];
        }
    }

    for (int i = 0; i < n; i++) { // идем по столбцам
        int pivot_row = find_pivot(&tmp, i);
        elementary_swap(&tmp, i, pivot_row, &sign);

        if (fabs(tmp.data[i][i]) < 1e-9) {//те вырождена
            free_matrix(&tmp);
            return 0.0;
        }

        for (int j = i + 1; j < n; j++) {// обнуляем под лиагональю
            double factor = tmp.data[j][i] / tmp.data[i][i];
            elementary_add(&tmp, j, i, factor, i);
        }
        det *= tmp.data[i][i];
    }

    double final_det = det * sign;
    free_matrix(&tmp);
    return final_det;
}




int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2 || n != m){
        return 1;
    }

    Matrix mat = init_matrix(n, n);
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            scanf("%lf", &mat.data[i][j]);
        }
    }

    printf("%.2lf\n", calculate_determinant(mat));
    free_matrix(&mat);
    return 0;
}




