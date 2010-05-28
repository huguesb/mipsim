
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *random_word();
void print_word(const char *w, const char *alphabet);

int main()
{
    int max_tries = 4;
    printf("Hangman\n");
    
    printf("choose difficulty (number of tries) : ");
    scanf("%d", &max_tries);
    
    char alphabet[26];
    
    srand(max_tries * (int)alphabet);
    
    while ( 1 )
    {
        printf("\nRejoice! A new word cometh to thou!\n");
        const char *w = random_word();
        
        char letter;
        int tries = max_tries;
        int found = 0;
        
        memset(alphabet, 0, 26);
        
        while ( tries )
        {
            letter = rand();
            
            print_word(w, alphabet);
            printf("    D-%3d     [", tries);
            
            for ( int i = 0; i < 26; ++i )
                printf("%c", alphabet[i] ? 'a' + i : ' ');
            
            printf("]\n? ");
            fflush(stdout);
            
            do {
                letter = fgetc(stdin);
            } while ( letter < 'a' || letter > 'z' );
            
            if ( alphabet[letter - 'a'] )
            {
                printf("\nMoron! Not much point sayin' the same thing twice, is there?\n");
            } else {
                alphabet[letter - 'a'] = 1;
                
                if ( strchr(w, letter) == NULL )
                {
                    printf("\nTough luck...\n");
                    --tries;
                } else {
                    // check for victory
                    const char *tmp = w;
                    
                    while ( *tmp )
                    {
                        if ( *tmp >= 'a' && *tmp <= 'z' && !alphabet[*tmp - 'a'] )
                            break;
                        
                        ++tmp;
                    }
                    
                    if ( !*tmp )
                    {
                        found = 1;
                        break;
                    }
                }
            }
        }
        
        if ( found )
            printf("\nHmph! Lesse if you can manage the next one...\n");
        else
            printf("\nHung high'n'short! Serves you right for being so dumb!\n");
    }
    
    return 0;
}

const char *dictionary[] = {
    "mips",
    "simulator",
    "instruction",
    "set",
    "executable",
    "linkable",
    "format",
    "relocation",
    "binary",
    "word",
    "half",
    "byte",
    "memory",
    "section",
    "symbol",
    "segment",
    "processor",
    "coprocessor",
    "register",
    "branch",
    "jump",
    "architecture",
    "stack",
    "heap",
    "delay",
    "slot",
    "monitor"
};

const char *random_word()
{
    const int dic_size = sizeof(dictionary) / sizeof(*dictionary);
    
    return dictionary[rand() % dic_size];
}

void print_word(const char *w, const char *alphabet)
{
    while ( *w )
    {
        printf("%c", (*w >= 'a') && (*w <= 'z') && !alphabet[*w - 'a'] ? '_' : *w);
        ++w;
    }
}
