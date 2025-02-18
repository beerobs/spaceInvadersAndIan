#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define LARGURA_JANELA 800
#define ALTURA_JANELA 600

#define STD_SIZE_X 32
#define STD_SIZE_Y 32

#define MAX_NAVES 168
#define MAX_LINHAS 8
#define MIN_LINHAS 4
#define MAX_NAVES_LINHA 21
#define MIN_NAVES_LINHA 7

#define LARGURA_BALA 10
#define ALTURA_BALA 15

// Cada linha possui no máximo 21 naves, e no mínimo 7.
// Máximo de 8 linhas

typedef struct Bala{
    Rectangle pos;
    Color color;
    int ativa;
    int tempo;
    int velocidade;
    Sound tiro;
}Bala;

typedef struct Nave{
    Rectangle pos;
    Color color;
    Bala bala;
    int velocidade;
    int direcao;
}Nave;

typedef struct Heroi{
    Rectangle pos;
    Color color;
    Bala bala;
    int velocidade;
    int direcao;
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
    Nave nave;
    Nave naves[MAX_NAVES];
    Heroi heroi;
    Bordas bordas[4];
    Assets assets;
    int alturaJanela;
    int larguraJanela;
    int tempoAnimação;
    int navesPorLinha;
    int linhasDeNaves;
    int quantidadeNaves;
}Jogo;

void IniciaJogo(Jogo *j);
void IniciaNaves(Jogo *j);
void AtualizaJogo(Jogo *j);
void DesenhaJogo(Jogo *j);
void AtualizaFrameDesenho(Jogo *j);
void AtualizaNavePos(Jogo *j);
void AtualizaHeroiPos(Jogo *j);
void DesenhaNaves(Jogo *j);
void DesenhaHeroi(Jogo *j);
void ColisaoBordas(Jogo *j);
void DesenhaBordas(Jogo *j);
int ColisaoBalas(Jogo *j);
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
void RandomizaPosicaoNaves(Jogo *j);




int main(){
    InitAudioDevice();
    
    srand(time(NULL));
    
    Jogo jogo;

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

    j->heroi.pos = (Rectangle) {LARGURA_JANELA/2 - STD_SIZE_X/2, ALTURA_JANELA - STD_SIZE_Y -10, STD_SIZE_X, STD_SIZE_Y};
    j->heroi.color = PURPLE;
    j->heroi.velocidade = 3;
    j->heroi.bala.ativa = 0;
    j->heroi.bala.tempo = GetTime();
    j->heroi.bala.velocidade = 10;
    j->heroi.bala.tiro = LoadSound("assets/shoot.wav");

    
    //IniciaNaves(j);
    j->nave.pos = (Rectangle) {0, 15, STD_SIZE_X, STD_SIZE_Y};
    j->nave.color = RED;
    j->nave.velocidade = 3;
    // direcao = 1 faz nave mover para direita, direcao = 0 faz mover para esquerda
    j->nave.direcao = 1;
    j->nave.bala.ativa = 0;
    j->nave.bala.tempo = GetTime();
    j->nave.bala.velocidade = 5;
    j->nave.bala.tiro = LoadSound("assets/shoot.wav");

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

    for(int i = 0; i < MAX_NAVES; i++){
        
        RandomizaPosicaoNaves(j);
        j->naves[i].color = RED;
        j->naves[i].velocidade = 3;
        /*direcao = 1 faz nave mover para direita, direcao = 0 faz mover para esquerda*/
        j->naves[i].direcao = 1;
        j->naves[i].bala.ativa = 0;
        j->naves[i].bala.tempo = GetTime();
        j->naves[i].bala.velocidade = 5;
        j->naves[i].bala.tiro = LoadSound("assets/shoot.wav");
    }


}

void RandomizaPosicaoNaves(Jogo *j){
    int navesPorLinha = (rand() % (MAX_NAVES_LINHA + 1 - MIN_NAVES_LINHA)) + MIN_NAVES_LINHA;
    int linhas = (rand() % (MAX_LINHAS + 1 - MIN_LINHAS)) + MIN_LINHAS;
    j->quantidadeNaves = navesPorLinha*linhas;
    
    // Posição aleatória onde a primeira coluna das naves irão começar.
    int xInicial = STD_SIZE_X * (rand() % (4 + (MAX_NAVES - navesPorLinha) + 1));
    // Posição aleatória onde a primeira linha das naves irão começar.
    int yInicial = STD_SIZE_Y * (rand() % 3 + 1);

    navesPorLinha = 3;
    linhas = 3;
    j->quantidadeNaves = 9;
    xInicial = STD_SIZE_X * 3;
    yInicial = STD_SIZE_Y * 3;

    for(int i = 0; i < linhas; i++){
        for(int k = 0; k < navesPorLinha; k++){
           int indice = k + navesPorLinha * i;
            j->naves[indice].pos = (Rectangle){(xInicial + STD_SIZE_X * k), (yInicial + STD_SIZE_Y * i), STD_SIZE_X, STD_SIZE_Y};
        }
    }
}

void AtualizaJogo(Jogo *j){
    AtualizaNavePos(j);
    AtualizaHeroiPos(j);
    AtiraBalas(j);
    AtiraBalasHeroi(j);
}

void DesenhaJogo(Jogo *j){
    BeginDrawing();
    ClearBackground(BLACK);
    DesenhaNaves(j);
    DesenhaHeroi(j);
    DesenhaBordas(j);
    EndDrawing();
}

int FinalDeJogo(Jogo* j){
    if (CheckCollisionRecs(j->nave.pos, j->heroi.bala.pos)) {
                DesenhaVitoria(j);
                return 1;
            }
    if (CheckCollisionRecs(j->heroi.pos, j->nave.bala.pos)) {
        DesenhaDerrota(j);
        return 1;
    }
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
    if(j->nave.direcao == 1){
        j->nave.pos.x += j->nave.velocidade;
    }else{
        j->nave.pos.x -= j->nave.velocidade;
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

void DesenhaNaves(Jogo *j){
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

    
    DrawTexturePro(j->assets.naveVerde, frameRecNave, (Rectangle){j->nave.pos.x, j->nave.pos.y, 32, 32},
    (Vector2){0, 0}, 0.0f, WHITE);

}

/* void DesenhaNaves(Jogo *j){
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

    for(int i = 0; i < j->quantidadeNaves; i++){
        DrawTexturePro(j->assets.naveVerde, frameRecNave, (Rectangle){j->naves[i].pos.x, j->naves[i].pos.y, 32, 32},
        (Vector2){0, 0}, 0.0f, WHITE);
    }

} */

void DesenhaHeroi(Jogo *j){
    DrawTexture(j->assets.heroiPrata, j->heroi.pos.x, j->heroi.pos.y, j->heroi.color);
}

void DesenhaBordas(Jogo *j){
    for(int i = 0; i < 4; i++){
        DrawRectangleRec(j->bordas[i].pos, LIGHTGRAY);
    }
}

void DesenhaBalas(Jogo *j){
    DrawRectangleRec(j->nave.bala.pos, YELLOW);
}

void DesenhaBalasHeroi(Jogo *j){
    DrawRectangleRec(j->heroi.bala.pos, RED);
}

void AtiraBalas(Jogo *j){
    if(j->nave.bala.ativa == 0 && GetTime()-j->nave.bala.tempo > 3){
        j->nave.bala.pos = (Rectangle){j->nave.pos.x+j->nave.pos.width/2, j->nave.pos.y+j->nave.pos.height/2, 
        LARGURA_BALA, ALTURA_BALA};
        j->nave.bala.ativa = 1;
        j->nave.bala.tempo = GetTime();
        PlaySound(j->nave.bala.tiro);
    }
    else if(ColisaoBalas(j)){
        j->nave.bala.ativa = 0;
        j->nave.bala.pos = (Rectangle){9999, 99999};
    }
    if(j->nave.bala.ativa == 1){
        j->nave.bala.pos.y += j->nave.bala.velocidade;
        DesenhaBalas(j);
    }
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
    if(CheckCollisionRecs(j->nave.pos, j->bordas[2].pos)){
        j->nave.direcao = 1;
    }else if(CheckCollisionRecs(j->nave.pos, j->bordas[3].pos)){
        j->nave.direcao = 0;
    }
}

int ColisaoBalas(Jogo *j){
    // Colisao bala com heroi
    if(CheckCollisionRecs(j->heroi.pos, j->nave.bala.pos)){
        return 1;
    }
    // Colisao bala com borda de baixo
    if(CheckCollisionRecs(j->nave.bala.pos, j->bordas[1].pos)){
        return 1;
    }
    return 0;
}

int ColisaoBalasHeroi(Jogo *j){
    // Colisao bala com nave
    if(CheckCollisionRecs(j->nave.pos, j->heroi.bala.pos)){
        return 1;
    }
    // Colisao bala com borda de baixo
    if(CheckCollisionRecs(j->heroi.bala.pos, j->bordas[0].pos)){
        return 1;
    }
    return 0;
}