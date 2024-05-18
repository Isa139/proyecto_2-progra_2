#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>

#define MAX_INGREDIENTS 10
#define MAX_CHARS 50

struct ingredient {
    char name[MAX_CHARS];
    int quantity;
    struct ingredient *next;
};

struct recipe {
    char name[MAX_CHARS];
    int price;
    int preparationTime;
    int consumptionTime;
    struct ingredient *ingredients;
    struct recipe *next;
};

struct inventory {
    char ingredient[MAX_CHARS];
    int amount;
    double price;
    struct inventory *next;
};

void freeIngredients(struct ingredient *head) {
    struct ingredient *current = head;
    while (current != NULL) {
        struct ingredient *temp = current;
        current = current->next;
        free(temp);
    }
}

void freeRecipes(struct recipe *head) {
    struct recipe *current = head;
    while (current != NULL) {
        struct recipe *temp = current;
        freeIngredients(temp->ingredients);
        current = current->next;
        free(temp); // Free the recipe
    }
}

struct recipe *readRecipes(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file %s\n", fileName);
        return NULL;
    }

    struct recipe *head = NULL;
    struct recipe *tail = NULL;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        struct recipe *newRecipe = (struct recipe *)malloc(sizeof(struct recipe));
        if (!newRecipe) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            fclose(file);
            freeRecipes(head);
            return NULL;
        }
        newRecipe->ingredients = NULL; // Initialize ingredients to NULL
        newRecipe->next = NULL; // Initialize next to NULL

        char *token = strtok(line, ",");
        if (!token) {
            free(newRecipe);
            continue;
        }
        strcpy(newRecipe->name, token);

        token = strtok(NULL, ",");
        if (token) newRecipe->price = atoi(token);

        token = strtok(NULL, ",");
        if (token) newRecipe->preparationTime = atoi(token);

        token = strtok(NULL, ",");
        if (token) newRecipe->consumptionTime = atoi(token);

        struct ingredient *lastIngredient = NULL;
        while ((token = strtok(NULL, ",")) && *token != '\n') {
            struct ingredient *newIngredient = (struct ingredient *)malloc(sizeof(struct ingredient));
            if (!newIngredient) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                freeIngredients(newRecipe->ingredients);
                free(newRecipe);
                fclose(file);
                freeRecipes(head);
                return NULL;
            }
            newIngredient->next = NULL; // Initialize next to NULL
            strcpy(newIngredient->name, token);

            token = strtok(NULL, ",");
            if (token) newIngredient->quantity = atoi(token);

            if (lastIngredient) {
                lastIngredient->next = newIngredient;
            } else {
                newRecipe->ingredients = newIngredient;
            }
            lastIngredient = newIngredient;
        }

        if (head == NULL) {
            head = newRecipe;
        } else {
            tail->next = newRecipe;
        }
        tail = newRecipe;
    }

    fclose(file);
    return head;
}

struct inventory *readInventory(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file %s\n", fileName);
        return NULL;
    }

    struct inventory *head = NULL;
    struct inventory *tail = NULL;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        struct inventory *newInventory = (struct inventory *)malloc(sizeof(struct inventory));
        if (!newInventory) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            fclose(file);
            return NULL;
        }
        newInventory->next = NULL; // Initialize next to NULL

        char *token = strtok(line, ",");
        if (!token) {
            free(newInventory);
            continue;
        }
        strcpy(newInventory->ingredient, token);

        token = strtok(NULL, ",");
        if (token) newInventory->amount = atoi(token);

        token = strtok(NULL, ",");
        if (token) newInventory->price = atof(token);

        if (head == NULL) {
            head = newInventory;
        } else {
            tail->next = newInventory;
        }
        tail = newInventory;
    }

    fclose(file);
    return head;
}

void printRecipes(struct recipe *head) {
    struct recipe *current = head;
    while (current != NULL) {
        printf("Recipe: %s\n", current->name);
        printf("Price: $%d\n", current->price);
        printf("Preparation Time: %d minutes\n", current->preparationTime);
        printf("Consumption Time: %d minutes\n", current->consumptionTime);

        printf("Ingredients:\n");
        struct ingredient *ingredientPtr = current->ingredients;
        while (ingredientPtr != NULL) {
            printf("- %s: %d\n", ingredientPtr->name, ingredientPtr->quantity);
            ingredientPtr = ingredientPtr->next;
        }
        printf("\n");

        current = current->next;
    }
}

void printInventory(struct inventory *head) {
    struct inventory *current = head;
    //while (current != NULL) {
        printf("Ingredient: %s\n", current->ingredient);
        printf("Price: $%f\n", current->price);
        printf("Amount: %d\n", current->amount);
        printf("\n");

        current = current->next;
    //}
}

char** getRecipeNames(struct recipe *head, int &count) {
    // First, count the number of recipes
    count = 0;
    struct recipe *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    // Allocate memory for the array of strings
    char **recipeNames = (char **)malloc(count * sizeof(char *));
    if (!recipeNames) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    // Populate the array with recipe names
    current = head;
    for (int i = 0; i < count; i++) {
        recipeNames[i] = (char *)malloc(MAX_CHARS * sizeof(char));
        if (!recipeNames[i]) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            // Free already allocated memory
            for (int j = 0; j < i; j++) {
                free(recipeNames[j]);
            }
            free(recipeNames);
            return NULL;
        }
        strcpy(recipeNames[i], current->name);
        current = current->next;
    }

    return recipeNames;
}

void initializeRandomSeed() {
    std::srand(std::time(nullptr));
}

int getRandomNumber(int count) {
    return std::rand() % count; // Index should be 0 to count-1
}

struct recipe* findRecipeByName(struct recipe* head, const char* recipeName) {
    struct recipe* current = head;
    while (current != NULL) {
        if (strcmp(current->name, recipeName) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL; // Return NULL if the recipe is not found
}

void alterInventory(struct inventory *inventoryHead, const char *ingredientName) {
    struct inventory *current = inventoryHead;

    // Traverse through each inventory item
    while (current != NULL) {
        // Check if the current inventory item matches the ingredient name
        if (strcmp(current->ingredient, ingredientName) == 0) {
            // Decrease the amount of the ingredient by 1
            current->amount -= 1;
            return; // Exit the function once the ingredient is found and updated
        }
        current = current->next; // Move to the next inventory item
    }

    // If the ingredient is not found in the inventory, you may choose to handle it accordingly
    fprintf(stderr, "Error: Ingredient '%s' not found in inventory.\n", ingredientName);
}


int main() {
    initializeRandomSeed(); // Call this once at the start of your program

    const char *fileNameR = "/Users/isagomez/Desktop/recipes.csv";
    const char *fileNameI = "/Users/isagomez/Desktop/inventory.csv";

    struct recipe *recipes = readRecipes(fileNameR);
    if (!recipes) {
        return 1;
    }

    struct inventory *inventory = readInventory(fileNameI);
    if (!inventory) {
        freeRecipes(recipes);
        return 1;
    }

    printRecipes(recipes);
    printInventory(inventory);

    // Example usage
    alterInventory(inventory, "Flour"); // Set the amount of Flour to 50



    printf("--------------Decrease:-------------\n");
    printInventory(inventory);

    freeRecipes(recipes);
    return 0;
}
