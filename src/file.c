#include "file.h"

#include <stdlib.h>

file *creer_file(void) {
    file *f = malloc(sizeof *f);
    if (f == NULL) return NULL;

    return f;
}

void liberer_file(file **f) {
    if ((*f) != NULL) {
        element *current = (*f)->first;
        while ((current = (*f)->first) != NULL) {
            (*f)->first = (*f)->first->next;
            free(current->node);
            free(current);
        }
    }
    free(*f);
    *f = NULL;
}

void enfiler(file *f, Node *node) {
    if (f == NULL) return NULL;

    element *new = malloc(sizeof *new);
    if (new == NULL) return NULL;

    new->node = node;
    new->next = NULL;

    if (f->first != NULL) {
        element *current = f->first;
        while (current->next != NULL) {
            current = current->next;
        }
        /* Ajout à la fin de la file */
        current->next = new;
    } else {
        f->first = new;
    }
}

Node *defiler(file *f) {
    if (f == NULL) return NULL;

    Node *node = NULL;

    if (f->first != NULL) {
        element *out = f->first;
        node = out->node;
        f->first = out->next;
        free(out->node);
        free(out);
    }
    return node;
}

void afficher_file(file *f) {
    if (f->first != NULL) {
        element *current = f->first;
        while (current->next != NULL) {
            printf("%i", current->node->symbol);
            current = current->next;
        }
    } else {
        printf("File vide.");
    }
}