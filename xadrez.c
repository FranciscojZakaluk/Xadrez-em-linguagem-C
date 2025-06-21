#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include <time.h>


#define ANSI_RESET   "\x1b[0m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_YELLOW  "\x1b[33m"
#define USERNAME_SIZE 50
#define MAX_PASSWORD_SIZE 20
#define FILENAME_USERS "usuarios.txt"

typedef struct {
    char username[USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];
} User;

typedef struct {
    char username[USERNAME_SIZE];
    int gamesPlayed;
    int wins;
    int draws;
    int losses;
} Stats;

User* users = NULL;
int userCount = 0;
int userCapacity = 2;

Stats* stats = NULL;
int statsCount = 0;
int statsCapacity = 2;


typedef struct {
    char sigla;
    char cor; 
    int movida; 
} Peca;

typedef struct {
    Peca* peca;
} Casa;

Casa* tabuleiro[8][8];

int abs(int x) {
    return x < 0 ? -x : x;
}

Peca* criar_peca(char sigla, char cor) {
    Peca* p = malloc(sizeof(Peca));
    p->sigla = sigla;
    p->cor = cor;
    p->movida = 0;
    return p;
}

void ocultar_senha(char* senha) {
    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r' && i < MAX_PASSWORD_SIZE - 1) {
        if (ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            senha[i++] = ch;
            printf("*");
        }
    }
    senha[i] = '\0';
    printf("\n");
}

void salvar_usuario(const char* nome, const char* senha) {
    FILE* f = fopen(FILENAME_USERS, "a");
    if (f) {
        fprintf(f, "%s,%s\n", nome, senha);
        fclose(f);
    }
}

bool verificar_credenciais(const char* nome, const char* senha) {
    FILE* f = fopen(FILENAME_USERS, "r");
    char linha[100];
    if (!f) return false;
    while (fgets(linha, sizeof(linha), f)) {
        char user[USERNAME_SIZE], pass[MAX_PASSWORD_SIZE];
        sscanf(linha, "%49[^,],%19s", user, pass);
        if (strcmp(user, nome) == 0 && strcmp(pass, senha) == 0) {
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}

void criar_usuario() {
    char nome[USERNAME_SIZE], senha[MAX_PASSWORD_SIZE];
    printf("Digite um nome de usuario: ");
    scanf(" %49s", nome);
    printf("Digite uma senha: ");
    ocultar_senha(senha);
    salvar_usuario(nome, senha);
    printf("Usuario criado com sucesso!\n");
}

bool fazer_login(char* nome_logado) {
    char nome[USERNAME_SIZE], senha[MAX_PASSWORD_SIZE];
    printf("Nome de usuario: ");
    scanf(" %49s", nome);
    printf("Senha: ");
    ocultar_senha(senha);

    if (verificar_credenciais(nome, senha)) {
        strcpy(nome_logado, nome);
        printf("Login bem-sucedido!\n");
        return true;
    } else {
        printf("Credenciais invalidas.\n");
        return false;
    }
}

void inicializar_tabuleiro() {
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++) {
            tabuleiro[i][j] = malloc(sizeof(Casa));
            tabuleiro[i][j]->peca = NULL;
        }

    // Pretas
    tabuleiro[0][0]->peca = criar_peca('T', 'P');
    tabuleiro[0][1]->peca = criar_peca('C', 'P');
    tabuleiro[0][2]->peca = criar_peca('B', 'P');
    tabuleiro[0][3]->peca = criar_peca('R', 'P');
    tabuleiro[0][4]->peca = criar_peca('K', 'P');
    tabuleiro[0][5]->peca = criar_peca('B', 'P');
    tabuleiro[0][6]->peca = criar_peca('C', 'P');
    tabuleiro[0][7]->peca = criar_peca('T', 'P');
    for (int j = 0; j < 8; j++)
        tabuleiro[1][j]->peca = criar_peca('P', 'P');

    // Brancas
    for (int j = 0; j < 8; j++)
        tabuleiro[6][j]->peca = criar_peca('P', 'B');
    tabuleiro[7][0]->peca = criar_peca('T', 'B');
    tabuleiro[7][1]->peca = criar_peca('C', 'B');
    tabuleiro[7][2]->peca = criar_peca('B', 'B');
    tabuleiro[7][3]->peca = criar_peca('R', 'B');
    tabuleiro[7][4]->peca = criar_peca('K', 'B');
    tabuleiro[7][5]->peca = criar_peca('B', 'B');
    tabuleiro[7][6]->peca = criar_peca('C', 'B');
    tabuleiro[7][7]->peca = criar_peca('T', 'B');
}

void imprimir_tabuleiro() {
    printf("\n    ");
    for (char c = 'A'; c <= 'H'; c++) printf(" %c  ", c);
    printf("\n");

    for (int i = 0; i < 8; i++) {
        printf("  +---+---+---+---+---+---+---+---+\n");
        printf("%d ", 8 - i);
        for (int j = 0; j < 8; j++) {
            Casa *casa = tabuleiro[i][j];
            printf("|");
            if (casa->peca) {
                const char *cor = (casa->peca->cor == 'B') ? ANSI_YELLOW : ANSI_RED;
                printf(" %s%c" ANSI_RESET " ", cor, casa->peca->sigla);
            } else {
                printf("   ");
            }
        }
        printf("| %d\n", 8 - i);
    }

    printf("  +---+---+---+---+---+---+---+---+\n");
    printf("    ");
    for (char c = 'A'; c <= 'H'; c++) printf(" %c  ", c);
    printf("\n\n");
}

int caminho_livre(int li, int ci, int lf, int cf) {
    int d_lin = (lf - li) ? (lf - li) / abs(lf - li) : 0;
    int d_col = (cf - ci) ? (cf - ci) / abs(cf - ci) : 0;
    int i = li + d_lin;
    int j = ci + d_col;

    while (i != lf || j != cf) {
        if (tabuleiro[i][j]->peca != NULL) return 0;
        i += d_lin;
        j += d_col;
    }
    return 1;
}

int movimento_valido(Peca* peca, int li, int ci, int lf, int cf) {
    int d_lin = lf - li;
    int d_col = cf - ci;
    Casa* destino = tabuleiro[lf][cf];

    if (peca->sigla == 'K') {
        if (abs(d_lin) <= 1 && abs(d_col) <= 1) return 1;


        if (peca->movida) return 0;
        if (d_lin != 0) return 0;

    
        if (d_col == 2 && tabuleiro[li][7]->peca && tabuleiro[li][7]->peca->sigla == 'T' && !tabuleiro[li][7]->peca->movida) {
            if (tabuleiro[li][5]->peca == NULL && tabuleiro[li][6]->peca == NULL)
                return 1;
        }

        
        if (d_col == -2 && tabuleiro[li][0]->peca && tabuleiro[li][0]->peca->sigla == 'T' && !tabuleiro[li][0]->peca->movida) {
            if (tabuleiro[li][1]->peca == NULL && tabuleiro[li][2]->peca == NULL && tabuleiro[li][3]->peca == NULL)
                return 1;
        }

        return 0;
    }

    switch (peca->sigla) {
        case 'P': 
            if (peca->cor == 'B') {
                if (d_col == 0 && d_lin == -1 && destino->peca == NULL) return 1;
                if (li == 6 && d_col == 0 && d_lin == -2 && tabuleiro[li - 1][ci]->peca == NULL && destino->peca == NULL) return 1;
                if (abs(d_col) == 1 && d_lin == -1 && destino->peca != NULL && destino->peca->cor != peca->cor) return 1; // captura peça adversária
            } else {
                if (d_col == 0 && d_lin == 1 && destino->peca == NULL) return 1;
                if (li == 1 && d_col == 0 && d_lin == 2 && tabuleiro[li + 1][ci]->peca == NULL && destino->peca == NULL) return 1;
                if (abs(d_col) == 1 && d_lin == 1 && destino->peca != NULL && destino->peca->cor != peca->cor) return 1; // captura peça adversária
            }
            return 0;
        case 'T': 
            return (d_lin == 0 || d_col == 0) && caminho_livre(li, ci, lf, cf);
        case 'B': 
            return abs(d_lin) == abs(d_col) && caminho_livre(li, ci, lf, cf);
        case 'C': 
            return (abs(d_lin) == 2 && abs(d_col) == 1) || (abs(d_lin) == 1 && abs(d_col) == 2);
        case 'R': 
            return ((abs(d_lin) == abs(d_col) || d_lin == 0 || d_col == 0) && caminho_livre(li, ci, lf, cf));
        default:
            return 0;
    }
}


void mover_peca(int li, int ci, int lf, int cf) {
    if (li < 0 || li >= 8 || ci < 0 || ci >= 8 || lf < 0 || lf >= 8 || cf < 0 || cf >= 8) {
        printf("Movimento invalido: coordenadas fora do tabuleiro.\n");
        return;
    }

    Casa* origem = tabuleiro[li][ci];
    Casa* destino = tabuleiro[lf][cf];

    if (!origem->peca) {
        printf("Movimento invalido: nao ha peça na origem.\n");
        return;
    }

    if (!movimento_valido(origem->peca, li, ci, lf, cf)) {
        printf("Movimento invalido para a peça %c.\n", origem->peca->sigla);
        return;
    }

    if (destino->peca && destino->peca->cor == origem->peca->cor) {
        printf("Movimento invalido: não pode capturar peça da mesma cor.\n");
        return;
    }

    // Roque
    if (origem->peca->sigla == 'K' && abs(cf - ci) == 2) {
        if (cf == 6) { 
            tabuleiro[lf][5]->peca = tabuleiro[lf][7]->peca;
            tabuleiro[lf][7]->peca = NULL;
            tabuleiro[lf][5]->peca->movida = 1;
        } else if (cf == 2) { 
            tabuleiro[lf][3]->peca = tabuleiro[lf][0]->peca;
            tabuleiro[lf][0]->peca = NULL;
            tabuleiro[lf][3]->peca->movida = 1;
        }
    }

    if (destino->peca) free(destino->peca);
    destino->peca = origem->peca;
    origem->peca = NULL;
    destino->peca->movida = 1;

    printf("Peca %c (%c) movida de [%d,%d] para [%d,%d]\n",
           destino->peca->sigla, destino->peca->cor, li, ci, lf, cf);
}

int converter_letra_para_indice(char c) {
    if (c >= 'A' && c <= 'H') return c - 'A';
    if (c >= 'a' && c <= 'h') return c - 'a';
    return -1;
}

int rei_em_xeque(char cor) {
    int i, j;
    int pos_rei_x = -1, pos_rei_y = -1;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if (tabuleiro[i][j]->peca && tabuleiro[i][j]->peca->sigla == 'K' && tabuleiro[i][j]->peca->cor == cor) {
                pos_rei_x = i;
                pos_rei_y = j;
                break;
            }
        }
        if (pos_rei_x != -1) break;
    }

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if (tabuleiro[i][j]->peca && tabuleiro[i][j]->peca->cor != cor) {
                if (movimento_valido(tabuleiro[i][j]->peca, i, j, pos_rei_x, pos_rei_y)) {
                    return 1;
                }
            }
        }
    }
    return 0; 
}


void jogada_bot() {
    typedef struct {
        int li, ci, lf, cf;
        int valor_captura; 
    } Movimento;

    Movimento movimentos_validos[256];
    int contador = 0;

    int valores_pecas[256];  
    valores_pecas['P'] = 10;
    valores_pecas['C'] = 300;
    valores_pecas['B'] = 300;
    valores_pecas['T'] = 500;
    valores_pecas['R'] = 1000;
    valores_pecas['K'] = 10000;

    for (int li = 0; li < 8; li++) {
        for (int ci = 0; ci < 8; ci++) {
            Casa* origem = tabuleiro[li][ci];
            if (!origem->peca || origem->peca->cor != 'P') continue;

            

            
            for (int lf = 0; lf < 8; lf++) {
    for (int cf = 0; cf < 8; cf++) {
        if (!movimento_valido(origem->peca, li, ci, lf, cf))
            continue;

        
        Casa* casa_destino = tabuleiro[lf][cf];
        if (casa_destino->peca && casa_destino->peca->cor == origem->peca->cor)
            continue;

        
        Peca* temp_dest = tabuleiro[lf][cf]->peca;
        tabuleiro[lf][cf]->peca = origem->peca;
        tabuleiro[li][ci]->peca = NULL;

        int em_xeque = rei_em_xeque('P');

        tabuleiro[li][ci]->peca = tabuleiro[lf][cf]->peca;
        tabuleiro[lf][cf]->peca = temp_dest;

        if (!em_xeque) {
            Movimento m;
            m.li = li; m.ci = ci; m.lf = lf; m.cf = cf;
            m.valor_captura = (temp_dest != NULL) ? valores_pecas[(int)temp_dest->sigla] : 0;
            movimentos_validos[contador++] = m;
                }
            }
        }
    }
}


    if (contador == 0) {
        printf("Bot não encontrou movimentos válidos.\n");
        return;
    }

    int melhor_valor = -1;
    int indice_melhor = 0;
    for (int i = 0; i < contador; i++) {
        if (movimentos_validos[i].valor_captura > melhor_valor) {
            melhor_valor = movimentos_validos[i].valor_captura;
            indice_melhor = i;
        }
    }

    

    if (melhor_valor == 0)
        indice_melhor = rand() % contador;

    Movimento m = movimentos_validos[indice_melhor];
    mover_peca(m.li, m.ci, m.lf, m.cf);
    printf("Bot moveu peça de %c%d para %c%d\n",
           m.ci + 'A', 8 - m.li, m.cf + 'A', 8 - m.lf);
}

int possui_movimentos_validos(char cor) {
    for (int li = 0; li < 8; li++) {
        for (int ci = 0; ci < 8; ci++) {
            Peca* p = tabuleiro[li][ci]->peca;
            if (!p || p->cor != cor) continue;

            for (int lf = 0; lf < 8; lf++) {
                for (int cf = 0; cf < 8; cf++) {
                    if (!movimento_valido(p, li, ci, lf, cf)) continue;

                    Casa* destino = tabuleiro[lf][cf];
                    if (destino->peca && destino->peca->cor == cor) continue;

                    // simula
                    Peca* temp = destino->peca;
                    tabuleiro[lf][cf]->peca = p;
                    tabuleiro[li][ci]->peca = NULL;
                    int em_xeque = rei_em_xeque(cor);
                    tabuleiro[li][ci]->peca = p;
                    tabuleiro[lf][cf]->peca = temp;

                    if (!em_xeque) return 1;
                }
            }
        }
    }
    return 0;
}

void verificar_xeque_ou_mate(char cor, int* fim_de_jogo) {
    if (rei_em_xeque(cor)) {
        if (!possui_movimentos_validos(cor)) {
            printf(ANSI_RED "Xeque-mate! Rei das %s foi capturado.\n" ANSI_RESET, cor == 'B' ? "Brancas" : "Pretas");
            *fim_de_jogo = 1;
        } else {
            printf(ANSI_RED "Rei das %s está em xeque!\n" ANSI_RESET, cor == 'B' ? "Brancas" : "Pretas");
        }
    } else if (!possui_movimentos_validos(cor)) {
        printf(ANSI_YELLOW "Empate! %s não tem movimentos válidos.\n" ANSI_RESET, cor == 'B' ? "Brancas" : "Pretas");
        *fim_de_jogo = 1;
    }
}



int main() {
    int opcao;
    char usuario_logado[USERNAME_SIZE];

    do {
        printf("\n====== MENU PRINCIPAL ======\n");
        printf("1. Criar usuario\n");
        printf("2. Fazer login\n");
        printf("3. Jogar contra bot\n");
        printf("4. Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
            case 1:
                criar_usuario();
                break;
            case 2:
                if (fazer_login(usuario_logado)) {
                    printf("Bem-vindo, %s!\n", usuario_logado);
                }
                break;
            case 3:
    if (strlen(usuario_logado) == 0) {
        printf("Voce precisa estar logado para jogar!\n");
    } else {
        inicializar_tabuleiro();
        char entrada[32];
        char origem[3], destino[3];
        char vez = 'B'; 

        srand(time(NULL)); 
        while (1) {
            imprimir_tabuleiro();

            if (vez == 'B') {
                printf("Sua vez (Brancas). Digite o movimento (ex. E2 E4) ou 'sair': ");
                fgets(entrada, sizeof(entrada), stdin);
                entrada[strcspn(entrada, "\n")] = 0;

                if (strcmp(entrada, "sair") == 0) break;

                if (sscanf(entrada, "%2s %2s", origem, destino) != 2) {
                    printf("Formato invalido. Use por exemplo: E2 E4\n");
                    continue;
                }

                int ci = converter_letra_para_indice(origem[0]);
                int li = 8 - (origem[1] - '0');
                int cf = converter_letra_para_indice(destino[0]);
                int lf = 8 - (destino[1] - '0');

                if (ci < 0 || cf < 0 || li < 0 || li >= 8 || lf < 0 || lf >= 8) {
                    printf("Coordenadas invalidas.\n");
                    continue;
                }

                Casa* casa_origem = tabuleiro[li][ci];
                if (!casa_origem->peca || casa_origem->peca->cor != vez) {
                    printf("Movimento invalido: escolha uma peça da sua cor.\n");
                    continue;
                }

                if (!movimento_valido(casa_origem->peca, li, ci, lf, cf)) {
                    printf("Movimento invalido para a peça %c.\n", casa_origem->peca->sigla);
                    continue;
                }

                
                Peca* peca_destino_original = tabuleiro[lf][cf]->peca;
                tabuleiro[lf][cf]->peca = casa_origem->peca;
                tabuleiro[li][ci]->peca = NULL;
                int rei_em_perigo = rei_em_xeque('B');
                tabuleiro[li][ci]->peca = tabuleiro[lf][cf]->peca;
                tabuleiro[lf][cf]->peca = peca_destino_original;

                if (rei_em_perigo) {
                    printf("Movimento invalido: seu rei ficaria em xeque.\n");
                    continue;
                }

                mover_peca(li, ci, lf, cf);
                vez = 'P';

                if (rei_em_xeque('P')) {
                    printf(ANSI_RED "Rei das Pretas está em xeque!\n" ANSI_RESET);
                }
            } else {
                printf("Vez do bot (Pretas)...\n");
                jogada_bot();
                vez = 'B';

                if (rei_em_xeque('B')) {
                    printf(ANSI_RED "Rei das Brancas está em xeque!\n" ANSI_RESET);
                }
            }
        }
        printf("Partida encerrada.\n");
    }
    break;

            case 4:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
    } while (opcao != 4);

    return 0;
}
