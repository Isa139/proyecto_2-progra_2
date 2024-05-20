#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <ctype.h> // for isspace

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

void freeInventory(struct inventory *head) {
    struct inventory *current = head;
    while (current != NULL) {
        struct inventory *temp = current;
        current = current->next;
        free(temp);
    }
}

char *trimWhitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    // All spaces?
    if (*str == 0) return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

void toLowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
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
        newRecipe->ingredients = NULL;
        newRecipe->next = NULL;

        char *token = strtok(line, ",");
        if (!token) {
            free(newRecipe);
            continue;
        }
        // Convert the recipe name to lowercase
        char *trimmedToken = trimWhitespace(token);
        strcpy(newRecipe->name, trimmedToken);
        toLowercase(newRecipe->name);

        token = strtok(NULL, ",");
        if (token) newRecipe->price = atoi(trimWhitespace(token));

        token = strtok(NULL, ",");
        if (token) newRecipe->preparationTime = atoi(trimWhitespace(token));

        token = strtok(NULL, ",");
        if (token) newRecipe->consumptionTime = atoi(trimWhitespace(token));

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
            newIngredient->next = NULL;

            // Trim whitespace around ingredient name and convert to lowercase
            char *ingredientName = trimWhitespace(token);
            toLowercase(ingredientName);
            strcpy(newIngredient->name, ingredientName);

            token = strtok(NULL, ",");
            if (token) newIngredient->quantity = atoi(trimWhitespace(token));

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
        newInventory->next = NULL;

        char *token = strtok(line, ",");
        if (!token) {
            free(newInventory);
            continue;
        }
        char *trimmedName = trimWhitespace(token);
        toLowercase(trimmedName);
        strcpy(newInventory->ingredient, trimmedName);

        token = strtok(NULL, ",");
        if (token) newInventory->amount = atoi(trimWhitespace(token));

        token = strtok(NULL, ",");
        if (token) newInventory->price = atof(trimWhitespace(token));

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
    while (current != NULL) {
        printf("Ingredient: %s\n", current->ingredient);
        printf("Price: $%.2f\n", current->price);
        printf("Amount: %d\n", current->amount);
        printf("\n");

        current = current->next;
    }
}

char** getRecipeNames(struct recipe *head, int &count) {
    count = 0;
    struct recipe *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    char **recipeNames = (char **)malloc(count * sizeof(char *));
    if (!recipeNames) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    current = head;
    for (int i = 0; i < count; i++) {
        recipeNames[i] = (char *)malloc(MAX_CHARS * sizeof(char));
        if (!recipeNames[i]) {
            fprintf(stderr, "Error: Memory allocation failed\n");
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
    return std::rand() % count;
}

struct recipe* findRecipeByName(struct recipe* head, const char* recipeName) {
    struct recipe* current = head;
    char lowercaseName[MAX_CHARS];
    strcpy(lowercaseName, recipeName);
    toLowercase(lowercaseName); // Convert to lowercase

    while (current != NULL) {
        if (strcmp(current->name, lowercaseName) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void alterInventory(struct inventory *inventoryHead, const char *ingredientName, int quantityChange) {
    struct inventory *current = inventoryHead;
    bool found = false;
    while (current != NULL) {
        if (strcmp(current->ingredient, ingredientName) == 0) {
            found = true;
            current->amount += quantityChange;
            return;
        }
        current = current->next;
    }

    if (!found) {
        fprintf(stderr, "Error: Ingredient '%s' not found in inventory.\n", ingredientName);
        exit(1);
    }
}

bool prepareRecipe(struct recipe *recipe, struct inventory *inventoryHead) {
    struct ingredient *currentIngredient = recipe->ingredients;

    // Check if all ingredients are available in the required quantity
    while (currentIngredient != NULL) {
        char ingredientName[MAX_CHARS];
        strcpy(ingredientName, currentIngredient->name); // Make a copy of the ingredient name
        trimWhitespace(ingredientName); // Trim whitespace
        printf("Checking ingredient: %s, quantity needed: %d\n", ingredientName, currentIngredient->quantity); // Debugging output
        struct inventory *currentInventory = inventoryHead;
        bool found = false;
        while (currentInventory != NULL) {
            char inventoryIngredientName[MAX_CHARS];
            strcpy(inventoryIngredientName, currentInventory->ingredient); // Make a copy of the inventory ingredient name
            trimWhitespace(inventoryIngredientName); // Trim whitespace
            if (strcmp(inventoryIngredientName, ingredientName) == 0) { // Compare trimmed names
                if (currentInventory->amount < currentIngredient->quantity) {
                    fprintf(stderr, "Error: Not enough %s in inventory for recipe %s\n", currentInventory->ingredient, recipe->name);
                    return false; // Not enough ingredients to prepare the recipe
                }
                found = true;
                break;
            }
            currentInventory = currentInventory->next;
        }
        if (!found) {
            fprintf(stderr, "Error: Ingredient %s not found in inventory for recipe %s\n", ingredientName, recipe->name);
            return false; // Ingredient not found in inventory
        }
        currentIngredient = currentIngredient->next;
    }

    // Deduct the required quantities from the inventory
    currentIngredient = recipe->ingredients;
    while (currentIngredient != NULL) {
        char ingredientName[MAX_CHARS];
        strcpy(ingredientName, currentIngredient->name); // Make a copy of the ingredient name
        trimWhitespace(ingredientName); // Trim whitespace
        toLowercase(ingredientName); // Convert to lowercase
        alterInventory(inventoryHead, ingredientName, -currentIngredient->quantity); // Alter inventory
        currentIngredient = currentIngredient->next;
    }

    printf("Recipe %s prepared successfully!\n\n", recipe->name);
    return true; // Successfully altered the inventory
}

int main() {
    initializeRandomSeed();

    const char *fileNameR = "/Users/isagomez/Documents/GitHub/proyecto_2-progra_2/data/recipes.csv";
    const char *fileNameI = "/Users/isagomez/Documents/GitHub/proyecto_2-progra_2/data/inventory.csv";

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

    // Example usage: Prepare Cinnamon Rolls
    struct recipe *recipeToPrepare = findRecipeByName(recipes, "Cinnamon Rolls");
    if (recipeToPrepare != NULL) {
        prepareRecipe(recipeToPrepare, inventory);
    } else {
        printf("Recipe not found\n");
    }

    // Example usage: Prepare Donuts
    recipeToPrepare = findRecipeByName(recipes, "Donuts");
    if (recipeToPrepare != NULL) {
        prepareRecipe(recipeToPrepare, inventory);
    } else {
        printf("Recipe not found\n");
    }

    // Example usage: Prepare Pancakes
    recipeToPrepare = findRecipeByName(recipes, "Pancakes");
    if (recipeToPrepare != NULL) {
        prepareRecipe(recipeToPrepare, inventory);
    } else {
        printf("Recipe not found\n\n");
    }

    // Example usage: Prepare Apple Pie
    recipeToPrepare = findRecipeByName(recipes, "Apple Pie");
    if (recipeToPrepare != NULL) {
        prepareRecipe(recipeToPrepare, inventory);
    } else {
        printf("Recipe not found\n");
    }

    printf("--------------Updated Inventory:-------------\n");
    printInventory(inventory);

    freeRecipes(recipes);
    freeInventory(inventory);
    return 0;
}
