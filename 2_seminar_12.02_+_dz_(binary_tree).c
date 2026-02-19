#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
    struct Node *parent;
    struct Node *left;
    struct Node *right;
    int val;
}Node;

typedef struct Tree{
    struct Node *root;
    int count;
}Tree;

void init_tree(Tree *tree){
    tree->root = NULL;
    tree->count = 0;
}

Node *create_node(int val){
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL){
        return NULL;
    }
    new_node->parent = NULL;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->val = val;
    return new_node;
}

int push_node(Tree *tree, int val) {
    Node *new_node = create_node(val);
    if (new_node == NULL) {
        return 0;
    }

    if (tree->root == NULL) {
        tree->root = new_node;
        tree->count++;
        return 1;
    }

    Node *current = tree->root;
    int inserted = 0;

    while (!inserted) {
        if (val < current->val) {
            if (current->left == NULL) {
                current->left = new_node;
                new_node->parent = current;
                inserted = 1;} 
            else {
                current = current->left;}
        } 
        
        else if (val > current->val) {
            if (current->right == NULL) {
                current->right = new_node;
                new_node->parent = current;
                inserted = 1;} 
            else {
                current = current->right;}
        } 
        
        else {
            free(new_node);
            return 0;
        }
    }

    tree->count++;
    return 1;
}

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

int get_heig(Node *node) {
    if (node == NULL) 
        return 0;
    return 1 + MAX(get_heig(node->left), get_heig(node->right));
}

// Функция поиска второго по величине элемента
int find_second_max(Tree *tree) {
    Node *max_node = tree->root;
    while (max_node->right != NULL) {
        max_node = max_node->right;
    }
    if (max_node->left != NULL) {
        Node *second_max = max_node->left;
        while (second_max->right != NULL) {
            second_max = second_max->right;
        }
        return second_max->val;
    }
    return max_node->parent->val;
}

// Функция для in-order обхода (вывод в порядке возрастания)
void output_in_oder(Node *node) {
    if (node != NULL) {
        output_in_oder(node->left);   // Левое поддерево
        printf("%d\n", node->val);     // Корень
        output_in_oder(node->right); // Правое поддерево
    }
}

// Функция для in-order обхода с выводом только листьев
void leaves(Node *node) {
    if (node != NULL) {
        leaves(node->left);  // Обходим левое поддерево

        // Проверяем, является ли узел листом
        if (node->left == NULL && node->right == NULL) {
            printf("%d\n", node->val);
        }

        leaves(node->right); // Обходим правое поддерево
    }
}

// Функция для in-order обхода с выводом только вершин с 2 потомками
void node_2_child(Node *node) {
    if (node != NULL) {
        node_2_child(node->left);  // Обходим левое поддерево

        // Проверяем, является ли вершиной с 2 потомками
        if (node->left != NULL && node->right != NULL) {
            printf("%d\n", node->val);
        }

        node_2_child(node->right); // Обходим правое поддерево
    }
}

// Функция для in-order обхода с выводом только вершин с 1 потомком
void node_1_child(Node *node) {
    if (node != NULL) {
        node_1_child(node->left);  // Обходим левое поддерево

        // Проверяем, является ли вершиной с 1 потомком
        if ((node->left != NULL && node->right == NULL)||(node->left == NULL && node->right != NULL)) {
            printf("%d\n", node->val);
        }

        node_1_child(node->right); // Обходим правое поддерево
    }
}


int main(void) {
    Tree tree;          
    init_tree(&tree);
    int val;
    int input = 0;

    while (!input) {
        scanf("%d", &val);
        if (val == 0) {
            input = 1; 
        } 
        else {
            push_node(&tree, val);
        }
    }

   
    int second_max = find_second_max(&tree);
   
    // output_in_oder(tree.root);
    // leaves(tree.root);
    // node_2_child(tree.root);
    // node_1_child(tree.root);

    // printf("Second max element %d\n", second_max); 
    // printf("Height: %d\n", get_heig(tree.root)); 
    // printf("Count: %d\n", tree.count);        
    

    return 0;
}


