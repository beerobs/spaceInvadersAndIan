#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define LARGURA_JANELA 800
#define ALTURA_JANELA 600
#define LARGURA_UTIL (LARGURA_JANELA-20)
#define ALTURA_UTIL (ALTURA_JANELA-20)

#define STD_SIZE_X 32
#define STD_SIZE_Y 32
#define ESPACAMENTO_NAVE_X 16
#define ESPACAMENTO_NAVE_Y 10
#define SPACE_X (STD_SIZE_X + ESPACAMENTO_NAVE_X)
#define SPACE_Y (STD_SIZE_Y + ESPACAMENTO_NAVE_Y)


#define MAX_LINHAS 9
#define MIN_LINHAS 4
#define MAX_NAVE_LINHA 12
#define MIN_NAVE_LINHA 7
#define MAX_NAVE MAX_LINHAS * MAX_NAVE_LINHA

#define LARGURA_BALA 10
#define ALTURA_BALA 15

#define QTD_BARREIRAS 3

/* 
for(int i = 0; i < j->linhasDeNaves; i++){
    for(int k = 0; k < j->navesPorLinha; k++){
            
    }
}
 */    

// Cada linha possui no máximo 21 nave, e no mínimo 7.
// Máximo de 8 linhas

typedef struct Bala{
    Rectangle pos;
    Color color;
    int ativa;
    int tempo;
    int velocidade;
}Bala;

typedef struct Barreira {
    Rectangle pos;
    int ativa;
    int vida;
} Barreira;

typedef struct BalaHeroi{
    Rectangle pos;
    Color color;
    int ativa;
    int tempo;
    int velocidade;
    Sound tiro;
}BalaHeroi;

typedef struct Naves{
    Color color;
    Bala bala;
    int velocidade;
    int direcao;
    Sound tiro;
}Naves;

typedef struct Nave{
    Rectangle pos;
    Color color;
    Bala bala;
    int ativa;
}Nave;

typedef struct Heroi{
    Rectangle pos;
    Color color;
    BalaHeroi bala;
    int velocidade;
    int direcao;
    int vidas;
}Heroi;

typedef struct Bordas{
    Rectangle pos;
} Bordas;

typedef struct Assets{
    Texture2D naveVerde;
    Texture2D heroiPrata;
    Texture2D telaFinal;
    Sound tiro;
}Assets;

typedef struct Jogo{
    Naves naves;
    Nave nave[MAX_LINHAS][MAX_NAVE_LINHA];
    Heroi heroi;
    Bordas bordas[4];
    Barreira barreiras[QTD_BARREIRAS];
    Assets assets;
    int alturaJanela;
    int larguraJanela;
    int tempoAnimação;
    int navesPorLinha;
    int linhasDeNaves;
    int qtdNave;
    int venceu;
    int perdeu;
}Jogo;

void IniciaJogo(Jogo *j);
void IniciaNaves(Jogo *j);
void AtualizaJogo(Jogo *j);
void DesenhaJogo(Jogo *j);
void AtualizaFrameDesenho(Jogo *j);
void AtualizaNavePos(Jogo *j);
void AtualizaHeroiPos(Jogo *j);
void DesenhaNave(Jogo *j);
void DesenhaHeroi(Jogo *j);
void ColisaoBordas(Jogo *j);
void DesenhaBordas(Jogo *j);
int ColisaoBalas(Jogo *j, int i, int k);
int ColisaoBalasHeroi(Jogo *j);
void DesenhaBordas(Jogo *j);
void AtiraBalas(Jogo *j);
void AtiraBalasHeroi(Jogo *j);
void CarregaImagens(Jogo *j);
void DescarregaImagens(Jogo *j);
void DesenhaBalas(Jogo *j);
void DesenhaBalasHeroi(Jogo *j);
int FinalDeJogo(Jogo* j);
void DesenhaVitoria(Jogo* j);
void DesenhaDerrota(Jogo* j);
void RandomizaPosicaoNave(Jogo *j);
void DesenhaVidas(Jogo *j);
void AtualizaBarreiras (Jogo *j);
void DesenhaBarreiras(Jogo *j) ;
void IniciaBarreiras(Jogo *j);
void SalvarRanking(int pontuacao, const char *nomeJogador);
int VerificaTop(int pontuacao);
void solicitarNome(char *nomeJogador);


int main(){
    InitAudioDevice();
    
    srand(time(NULL));
    
    Jogo jogo = {};

    jogo.alturaJanela = ALTURA_JANELA;
    jogo.larguraJanela = LARGURA_JANELA;

    InitWindow(jogo.larguraJanela, jogo.alturaJanela, "Space Invaders");
    SetTargetFPS(60);
    IniciaJogo(&jogo);
    CarregaImagens(&jogo);
    Music musicaJogo = LoadMusicStream("assets/musica.mp3");
    PlayMusicStream(musicaJogo);

    while(!WindowShouldClose() && !FinalDeJogo(&jogo)){
        UpdateMusicStream(musicaJogo);
        AtualizaFrameDesenho(&jogo);
    }

    UnloadMusicStream(musicaJogo);
    DescarregaImagens(&jogo);
    CloseWindow();
    return 0;
}

void IniciaJogo(Jogo *j){
    j->tempoAnimação = GetTime();
    j->venceu = 0;
    j->perdeu = 0;

    IniciaBarreiras(j);

    j->heroi.pos = (Rectangle) {LARGURA_JANELA/2 - STD_SIZE_X/2, ALTURA_JANELA - STD_SIZE_Y -10, STD_SIZE_X, STD_SIZE_Y};
    j->heroi.color = PURPLE;
    j->heroi.velocidade = 3;
    j->heroi.bala.ativa = 0;
    j->heroi.bala.tempo = GetTime();
    j->heroi.bala.velocidade = 10;
    j->heroi.bala.tiro = LoadSound("assets/shoot.wav");
    j->heroi.vidas = 3;
    
    IniciaNaves(j);

    //borda encima
    j->bordas[0].pos = (Rectangle){0, 0, LARGURA_JANELA, 10};
    //borda embaixo
    j->bordas[1].pos = (Rectangle){0, ALTURA_JANELA-10, LARGURA_JANELA, 10};
    //borda esquerda
    j->bordas[2].pos = (Rectangle){0, 0, 10, ALTURA_JANELA};
    //borda direita
    j->bordas[3].pos = (Rectangle){LARGURA_JANELA-10, 0, 10, ALTURA_JANELA};
}

void IniciaNaves(Jogo *j){
    
    j->navesPorLinha = MIN_LINHAS + rand() % (MAX_NAVE_LINHA - MIN_NAVE_LINHA + 1);
    j->linhasDeNaves = MIN_LINHAS + rand() % (MAX_LINHAS - MIN_LINHAS + 1);
    j->qtdNave = j->navesPorLinha * j->linhasDeNaves;
    
    //Inicializa a matriz de naves
    int numAleatorio = 0;
    int numAnterior = -1;
    Color corAleatoria = {};
    for(int i = 0; i < j->linhasDeNaves; i++){
        
        //Estabelece a cor aleatória da linha de naves, não repetindo a cor da linha anterior.
        while(numAleatorio == numAnterior) numAleatorio = rand() % 4;
        numAnterior = numAleatorio;
        if(numAleatorio == 3) corAleatoria = GRAY;
        else if(numAleatorio == 2) corAleatoria = BLUE;
        else if(numAleatorio == 1) corAleatoria = PINK;
        else if(numAleatorio == 0) corAleatoria = PURPLE;
        
        for(int k = 0; k < j->navesPorLinha; k++){
            j->nave[i][k].color = corAleatoria;
            j->nave[i][k].ativa = 1;
            j->nave[i][k].bala.ativa = 0;
            j->nave[i][k].bala.tempo = GetTime();
        }
    }

    RandomizaPosicaoNave(j);

    j->naves.velocidade = 3;
    /*direcao = 1 faz nave mover para direita, direcao = 0 faz mover para esquerda*/
    j->naves.direcao = 1;
    j->naves.bala.ativa = 0;
    j->naves.bala.velocidade = 5;
    j->naves.tiro = LoadSound("assets/shoot.wav");
}

void RandomizaPosicaoNave(Jogo *j){
    int posPrimeiraColuna = 16;
    // Faz com que o bloco de nave fique acima de SPACE_Y*4, + SPACE*Y * (até j->linhasDeNaves não ocupadas) + 
    // SPACE_Y * (j->linhasDeNaves ocupadas) para garantir que a última linha fique em baixo
    int posPrimeiraLinha  = (ALTURA_JANELA-10) - (SPACE_Y*4 + SPACE_Y*(rand() % (9 - j->linhasDeNaves + 1)) + SPACE_Y * j->linhasDeNaves);    

    for(int i = 0; i < j->linhasDeNaves; i++){
        for(int k = 0; k < j->navesPorLinha; k++){
           int indice = k + j->navesPorLinha * i;
            j->nave[i][k].pos = (Rectangle){(posPrimeiraColuna + ESPACAMENTO_NAVE_X*k + STD_SIZE_X * k), (posPrimeiraLinha + ESPACAMENTO_NAVE_Y*i + STD_SIZE_Y * i), STD_SIZE_X, STD_SIZE_Y};
        }
    }
}

void AtualizaJogo(Jogo *j){
    AtualizaNavePos(j);
    AtualizaHeroiPos(j);
    AtualizaBarreiras(j);
    AtiraBalas(j);
    AtiraBalasHeroi(j);
    AtualizaBarreiras(j);
}

void DesenhaJogo(Jogo *j){
    BeginDrawing();
    ClearBackground(BLACK);
    DesenhaNave(j);
    DesenhaHeroi(j);
    DesenhaBordas(j);
    DesenhaBarreiras(j);
    DesenhaVidas(j);
    EndDrawing();
}

int FinalDeJogo(Jogo* j){
    
    for(int i = 0; i < j->linhasDeNaves; i++){
        for(int k = 0; k < j->navesPorLinha; k++){
            if (j->qtdNave <= 0) {
                    DesenhaVitoria(j);
                    return 1;
                }
            if (j->heroi.vidas <= 0) {
                if (VerificaTop(j->pontuacao)) {
                    char nomeJogador[50];
                    solicitarNome(nomeJogador);
                    SalvarRanking(j->pontuacao, nomeJogador);
                }
                DesenhaDerrota(j);
                return 1;
        }
        }
    }

    return 0;
}   

void DesenhaVitoria(Jogo* j){
    j->assets.telaFinal = LoadTexture("assets/YouWin.png");
        while(IsKeyUp(KEY_ESCAPE) && !WindowShouldClose()){
            BeginDrawing();
            DrawTexture(j->assets.telaFinal, 0, 0, WHITE);
            EndDrawing();
        }
}

void DesenhaDerrota(Jogo* j){
    j->assets.telaFinal = LoadTexture("assets/YouLose.png");
        while(IsKeyUp(KEY_ESCAPE) && !WindowShouldClose()){
            BeginDrawing();
            DrawTexture(j->assets.telaFinal, 0, 0, WHITE);
            EndDrawing();
        }
}

void AtualizaFrameDesenho(Jogo *j){
    AtualizaJogo(j);
    DesenhaJogo(j);
}

void AtualizaNavePos(Jogo *j){
    ColisaoBordas(j);

    if(j->naves.direcao == 1){
        
        for(int i = 0; i < j->linhasDeNaves; i++){

            for(int k = 0; k < j->navesPorLinha; k++){
                j->nave[i][k].pos.x += j->naves.velocidade;
            }

        }

    }else {
        
        for(int i = 0; i < j->linhasDeNaves; i++){
            for(int k = 0; k < j->navesPorLinha; k++){
                j->nave[i][k].pos.x -= j->naves.velocidade;
            }
        }
    
    }
}

void AtualizaHeroiPos(Jogo *j){
    if(IsKeyDown(KEY_RIGHT) && j->heroi.pos.x < LARGURA_JANELA-STD_SIZE_X - 10){
        j->heroi.pos.x += j->heroi.velocidade;
    }else if(IsKeyDown(KEY_LEFT) && j->heroi.pos.x > 10){
        j->heroi.pos.x -= j->heroi.velocidade;
    }
}

void CarregaImagens(Jogo *j){
    j->assets.naveVerde = LoadTexture("assets/GreenAnimation.png");
    j->assets.heroiPrata = LoadTexture("assets/SilverStatic.png");
}

void DescarregaImagens(Jogo *j){
    UnloadTexture(j->assets.naveVerde);
    UnloadTexture(j->assets.heroiPrata);
}

void DesenhaNave(Jogo *j){
    Vector2 tamanhoFrame = {32, 32};
    
    static Vector2 frame = {0, 0};
    static float tempoUltimaTroca = 0;
    
    if(GetTime() - tempoUltimaTroca >= 1){
        if(frame.x == 0){
            frame.x = 1;
        }else{
            frame.x = 0;
        }

        tempoUltimaTroca = GetTime();
    }
    Rectangle frameRecNave = {frame.x * tamanhoFrame.x, frame.y*tamanhoFrame.y,
    tamanhoFrame.x, tamanhoFrame.y};

    for(int i = 0; i < j->linhasDeNaves; i++){
        for(int k = 0; k < j->navesPorLinha; k++)
            if(j->nave[i][k].ativa)
                DrawTexturePro(
                    j->assets.naveVerde, 
                    frameRecNave, 
                    (Rectangle){j->nave[i][k].pos.x, j->nave[i][k].pos.y, 32, 32}, 
                    (Vector2){0, 0}, 
                    0.0f, 
                    j->nave[i][k].color
            );
    }

}

void DesenhaHeroi(Jogo *j){
    DrawTexture(j->assets.heroiPrata, j->heroi.pos.x, j->heroi.pos.y, j->heroi.color);
}

void DesenhaBordas(Jogo *j){
    for(int i = 0; i < 4; i++){
        DrawRectangleRec(j->bordas[i].pos, LIGHTGRAY);
    }
}

void DesenhaBalas(Jogo *j){
    for(int i = 0; i < j->linhasDeNaves; i++){
        for(int k = 0; k < j->navesPorLinha; k++){
            DrawRectangleRec(j->nave[i][k].bala.pos, YELLOW);            
        }
    }
}

void DesenhaBalasHeroi(Jogo *j){
    DrawRectangleRec(j->heroi.bala.pos, RED);
}

void AtiraBalas(Jogo *j){
    for(int k = 0; k < j->navesPorLinha; k++){
            for(int i = j->navesPorLinha-1; i >= 0; i--){
                if(i != j->navesPorLinha-1 && j->nave[i+1][k].ativa){
                    break;
                }
                if(j->nave[i][k].ativa && !j->nave[i][k].bala.ativa && GetTime()-j->nave[i][k].bala.tempo > 3){
                    j->nave[i][k].bala.pos = (Rectangle){j->nave[i][k].pos.x+j->nave[i][k].pos.width/2, j->nave[i][k].pos.y+j->nave[i][k].pos.height/2, 
                    LARGURA_BALA, ALTURA_BALA};
                    j->nave[i][k].bala.ativa = 1;
                    j->nave[i][k].bala.tempo = GetTime();
                    PlaySound(j->naves.tiro);
                }
                else if(ColisaoBalas(j, i, k)){
                    j->nave[i][k].bala.ativa = 0;
                    j->nave[i][k].bala.pos = (Rectangle){9999, 99999};
                }
                if(j->nave[i][k].bala.ativa == 1){
                    j->nave[i][k].bala.pos.y += j->naves.bala.velocidade;
                }
            }
        }

        DesenhaBalas(j);
}

void AtiraBalasHeroi(Jogo *j){
    if(j->heroi.bala.ativa == 0 && IsKeyDown(KEY_SPACE)){
        j->heroi.bala.pos = (Rectangle){j->heroi.pos.x+j->heroi.pos.width/2, j->heroi.pos.y+j->heroi.pos.height/2, 
        LARGURA_BALA, ALTURA_BALA};
        j->heroi.bala.ativa = 1;
        j->heroi.bala.tempo = GetTime();
        PlaySound(j->heroi.bala.tiro);
    }
    else if(ColisaoBalasHeroi(j)){
        j->heroi.bala.ativa = 0;
        j->heroi.bala.pos = (Rectangle){999, 999};
    }
    if(j->heroi.bala.ativa == 1){
        j->heroi.bala.pos.y -= j->heroi.bala.velocidade;
        DesenhaBalasHeroi(j);
    }
}

void ColisaoBordas(Jogo *j){
    
    if(CheckCollisionRecs(j->nave[0][0].pos, j->bordas[2].pos)){
        j->naves.direcao = 1;
    }else if(CheckCollisionRecs(j->nave[0][j->navesPorLinha-1].pos, j->bordas[3].pos)){
        j->naves.direcao = 0;
    }
}

int ColisaoBalas(Jogo *j, int i, int k){
    
    if(CheckCollisionRecs(j->heroi.pos, j->nave[i][k].bala.pos)){
        j->heroi.vidas--;
        return 1;
    }
    // Colisao bala com borda de baixo
    if(CheckCollisionRecs(j->nave[i][k].bala.pos, j->bordas[1].pos)){
        return 1;
    }

    return 0;
}

int ColisaoBalasHeroi(Jogo *j){
    // Colisao bala com as naves
    for(int i = 0; i < j->linhasDeNaves; i++){
        for(int k = 0; k < j->navesPorLinha; k++){
            if(CheckCollisionRecs(j->nave[i][k].pos, j->heroi.bala.pos) && j->nave[i][k].ativa){
                j->nave[i][k].ativa = 0;
                j->qtdNave--;
                return 1;
            }
        }
    }

    // Colisao bala com borda de cima
    if(CheckCollisionRecs(j->heroi.bala.pos, j->bordas[0].pos)){
        return 1;
    }
    
    for (int i = 0; i < 3; i++) {
        if (CheckCollisionRecs(j->heroi.bala.pos, j->barreiras[i].pos) && j->barreiras[i].ativa) {
            j->heroi.bala.ativa = 0;
            j->heroi.bala.pos = (Rectangle){986, 1063};
        }
    }
    return 0;
}

void AtualizaBarreiras (Jogo *j){
    for(int i = 0; i < j->linhasDeNaves; i++){
        for(int k = 0; k < j->navesPorLinha; k++){
            for (int b = 0; b < 3; b++) {
                if (j->barreiras[b].ativa && CheckCollisionRecs(j->nave[i][k].bala.pos, j->barreiras[b].pos) && j->nave[i][k].bala.ativa) {
                    j->nave[i][k].bala.ativa = 0;
                    j->nave[i][k].bala.pos.x = 999;
                    j->nave[i][k].bala.pos.y = 999;
                    j->barreiras[b].vida--;
                
                    if (j->barreiras[b].vida == 0) {
                        j->barreiras[b].ativa = 0;
                        j->nave[i][k].bala.ativa = 0;
                    }
                }
        }
        }
    }
    
    
}

void DesenhaVidas(Jogo *j) {
    char textoVidas[20];
    sprintf(textoVidas, "V: %d", j->heroi.vidas);
    DrawText(textoVidas, 10, 10, 20, LIGHTGRAY);
}

void DesenhaBarreiras(Jogo *j) {
    for (int i = 0; i < 3; i++) {
        if (j->barreiras[i].ativa) {
            DrawRectangleRec(j->barreiras[i].pos, BLUE);
        }
    }
}

void IniciaBarreiras(Jogo *j) {
    j->barreiras[0].pos = (Rectangle) {150 - 75 / 2, ALTURA_JANELA / 2 + 222, 75, 15};
    j->barreiras[1].pos = (Rectangle) {400 - 75 / 2, ALTURA_JANELA / 2 + 222, 75, 15};
    j->barreiras[2].pos = (Rectangle) {650 - 75 / 2, ALTURA_JANELA / 2 + 222, 75, 15};
    for (int i = 0; i < 3; i++) {
        j->barreiras[i].ativa = 1;
        j->barreiras[i].vida = 3;
    }
}