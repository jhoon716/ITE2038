#include "bpt.h"

// MAIN

int main( int argc, char ** argv ) {

    char * input_file;
    FILE * fp;
    node * root;
    int input, range2;
    char input2[120];
    char instruction;
    char license_part;
    char pathname[150];
    int result;

    char * input3 = (char *) malloc(sizeof(char) * 120);

    root = NULL;
    verbose_output = false;

    /*
    if (argc > 1) {
        order = atoi(argv[1]);
        if (order < MIN_ORDER || order > MAX_ORDER) {
            fprintf(stderr, "Invalid order: %d .\n\n", order);
            usage_3();
            exit(EXIT_FAILURE);
        }
    }
    */

    license_notice();
    usage_1();  
    usage_2();

    if (argc > 1) {
        if (open_db(argv[1]) != 0) {
            perror("Failure open db file.");
        }
    }

    if (argc > 2) {
        input_file = argv[2];
        fp = fopen(input_file, "r");
        if (fp == NULL) {
            perror("Failure  open input file.");
            exit(EXIT_FAILURE);
        }
        while (!feof(fp)) {
            fscanf(fp, "%d %s\n", &input, input3);
            insert(input, input3);
        }
        fclose(fp);
        print_tree();
    }


    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'o':
            scanf("%s", pathname);
            if (open_db(pathname) != 0) {
                perror("Failure open db file.");
            }
            break;
        case 'd':
            scanf("%d", &input);
            result = delete(input);
            if (result == 0) 
                printf("Deletion success.\n");
            else if (result == -1)
                printf("Failure deletion : value with key %d not found.\n", input);
            else 
                perror("Failure deletion.");
            print_tree();
            break;
        case 'i':
            scanf("%d %s", &input, input3);
            //printf("%d\n", insert(input, input2));
            insert(input, input3);
            print_tree();
            break;
        case 'f':
            scanf("%d", &input);
            find_and_print(input);
            break;
        //case 'p':
        case 'r':
            scanf("%d %d", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            //find_and_print_range(root, input, range2, instruction == 'p');
            break;
        case 'l':
            print_leaves();
            break;
        case 'q':
            while (getchar() != (int)'\n');
            fclose(data_file);
            return EXIT_SUCCESS;
            break;
        case 't':
            print_tree();
            break;
        case 'v':
            verbose_output = !verbose_output;
            break;
        /*
        case 'x':
            if (root)
                root = destroy_tree(root);
            print_tree(root);
            break;
        */
        default:
            usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return EXIT_SUCCESS;
}
