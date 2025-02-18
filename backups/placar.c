#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define EPSILON 1e-9;

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


#define MAX_LINHAS 6
#define MIN_LINHAS 4
#define MAX_NAVE_LINHA 9
#define MIN_NAVE_LINHA 7
#define MAX_NAVE MAX_LINHAS * MAX_NAVE_LINHA

#define CHANCE_DE_TIRO 20
#define LARGURA_BALA 10
#define ALTURA_BALA 15

#define QTD_BARREIRAS 3

/* 
for(int i = 0; i < j->rodada.linhasDeNaves; i++){
    for(int k = 0; k < j->rodada.navesPorLinha; k++){
            
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
    int intervalo_de_tiro;
    int velocidade;
}Bala;

typedef struct Barreira {
    Rectangle pos;
    Color cor;
    int ativa;
    int vida;
    int direcao;
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
    double velocidade;
    int direcao;
    Sound tiro;
}Naves;

typedef struct Nave{
    Rectangle pos;
    Color color;
    Bala bala;
    int ativa;
    int chanceDeTiro;
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

typedef struct Rodada{
    int numeroRodadas;
    double dificuldade;
    int navesPorLinha;
    int linhasDeNaves;
    int qtdNave;

    double maxTempoRodada;
    double inicioRodada;

}Rodada;

typedef struct Jogo{
    Naves naves;
    Nave nave[MAX_LINHAS][MAX_NAVE_LINHA];
    Heroi heroi;
    Bordas bordas[4];
    Rodada rodada;
    Barreira barreiras[QTD_BARREIRAS];
        int decrementoCorBarreira;
    Assets assets;

    int alturaJanela;
    int larguraJanela;

    int tempoAnimação;
    int pontuacao;

}Jogo;

void IniciaJogo(Jogo *j);
void IniciaNaves(Jogo *j);
void AtualizaJogo(Jogo *j);
void DesenhaJogo(Jogo *j);
void AtualizaFrameDesenho(Jogo *j);
void AtualizaRodada(Jogo* j);
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
void DesenhaRodada(Jogo* j);
void DesenhaTempo(Jogo *j);
void AtualizaBarreiras (Jogo *j);
void DesenhaBarreiras(Jogo *j) ;
void IniciaBarreiras(Jogo *j, int navesNoMapa);
void DesenhaPontuacao(Jogo* j);
int DoubleEquals(double a, double b);
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
    j->rodada.numeroRodadas = 1;
    j->rodada.dificuldade = 1;
    j->rodada.maxTempoRodada = 60;
    j->pontuacao = 0;

    j->heroi.pos = (Rectangle) {LARGURA_JANELA/2 - STD_SIZE_X/2, ALTURA_JANELA - STD_SIZE_Y -10, STD_SIZE_X, STD_SIZE_Y};
    j->heroi.color = PURPLE;
    j->heroi.velocidade = 3;
    j->heroi.bala.ativa = 0;
    j->heroi.bala.tempo = GetTime();
    j->heroi.bala.velocidade = 30;
    j->heroi.bala.tiro = LoadSound("assets/shoot.wav");
    j->heroi.vidas = 3;
    
    IniciaNaves(j);
    IniciaBarreiras(j,j->rodada.navesPorLinha);

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
    
    j->rodada.navesPorLinha = GetRandomValue(MIN_NAVE_LINHA, MAX_NAVE_LINHA); //MIN_LINHAS + rand() % (MAX_NAVE_LINHA - MIN_NAVE_LINHA + 1);
    j->rodada.linhasDeNaves = GetRandomValue(MIN_LINHAS, MAX_LINHAS); //MIN_LINHAS + rand() % (MAX_LINHAS - MIN_LINHAS + 1);
    j->rodada.qtdNave = j->rodada.navesPorLinha * j->rodada.linhasDeNaves;
    
    //Inicializa a matriz de naves
    int numAleatorio = 0;
    int numAnterior = -1;
    Color corAleatoria = {};
    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        
        //Estabelece a cor aleatória da linha de naves, não repetindo a cor da linha anterior.
        while(numAleatorio == numAnterior) 
            numAleatorio = rand() % 4;
        if(numAleatorio == 3)
            corAleatoria = GRAY;
        else if(numAleatorio == 2) 
            corAleatoria = BLUE;
        else if(numAleatorio == 1) 
            corAleatoria = PINK;
        else if(numAleatorio == 0) 
            corAleatoria = PURPLE;
        numAnterior = numAleatorio;
        
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
            j->nave[i][k].color = corAleatoria;
            j->nave[i][k].ativa = 1;
            j->nave[i][k].bala.ativa = 0;
            j->nave[i][k].chanceDeTiro = CHANCE_DE_TIRO + j->rodada.dificuldade * (1.2 + j->rodada.linhasDeNaves - i);
            j->nave[i][k].bala.tempo = GetTime();
        }
    }

    RandomizaPosicaoNave(j);

    j->naves.velocidade = 1 + (double)j->rodada.dificuldade/4;
    /*direcao = 1 faz nave mover para direita, direcao = 0 faz mover para esquerda*/
    j->naves.direcao = 1;
    j->naves.bala.ativa = 0;
    j->naves.bala.velocidade = 5;
    if(j->rodada.numeroRodadas == 1)
        j->naves.tiro = LoadSound("assets/shoot.wav");
}

void RandomizaPosicaoNave(Jogo *j){
    int posPrimeiraColuna = 16;
    // Faz com que o bloco de nave fique acima de SPACE_Y*4, + SPACE*Y * (até j->rodada.linhasDeNaves não ocupadas) + 
    // SPACE_Y * (j->rodada.linhasDeNaves ocupadas) para garantir que a última linha fique em baixo
    //int posPrimeiraLinha  = (ALTURA_JANELA-10) - (SPACE_Y*4 + SPACE_Y*(rand() % (9 - j->rodada.linhasDeNaves + 1)) + SPACE_Y * j->rodada.linhasDeNaves);    
    int posPrimeiraLinha  = (ALTURA_JANELA-10) - (SPACE_Y*4.5 + SPACE_Y * (j->rodada.linhasDeNaves + GetRandomValue(0, 7 - j->rodada.linhasDeNaves)));    

    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
           int indice = k + j->rodada.navesPorLinha * i;
            j->nave[i][k].pos = (Rectangle){(posPrimeiraColuna + ESPACAMENTO_NAVE_X*k + STD_SIZE_X * k), (posPrimeiraLinha + ESPACAMENTO_NAVE_Y*i + STD_SIZE_Y * i), STD_SIZE_X, STD_SIZE_Y};
        } 
    }
}

void AtualizaJogo(Jogo *j){
    AtualizaRodada(j);
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
    DesenhaTempo(j);
    DesenhaPontuacao(j);
    DesenhaRodada(j);
    EndDrawing();
}

int FinalDeJogo(Jogo* j){
    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
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

void AtualizaRodada(Jogo *j){
    if(j->rodada.qtdNave<=0 || (GetTime() - j->rodada.inicioRodada) > j->rodada.maxTempoRodada){
        j->rodada.numeroRodadas++;
        j->rodada.maxTempoRodada = j->rodada.maxTempoRodada * 0.90 >= 10 ? j->rodada.maxTempoRodada * 0.90 : 10;
        j->rodada.inicioRodada = GetTime();
        j->rodada.dificuldade += 1;
        IniciaNaves(j);
        IniciaBarreiras(j, j->rodada.navesPorLinha);
    }
    return;
}

void AtualizaFrameDesenho(Jogo *j){
    AtualizaRodada(j);
    AtualizaJogo(j);
    DesenhaJogo(j);
}

void AtualizaNavePos(Jogo *j){
    ColisaoBordas(j);

    if(j->naves.direcao == 1){
        
        for(int i = 0; i < j->rodada.linhasDeNaves; i++){

            for(int k = 0; k < j->rodada.navesPorLinha; k++){
                j->nave[i][k].pos.x += j->naves.velocidade;
            }

        }

    }else {
        for(int i = 0; i < j->rodada.linhasDeNaves; i++){
            for(int k = 0; k < j->rodada.navesPorLinha; k++){
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

    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++)
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
    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
            if(j->nave[i][k].bala.ativa)
                DrawRectangleRec(j->nave[i][k].bala.pos, YELLOW);            
        }
    }
}

void DesenhaBalasHeroi(Jogo *j){
    DrawRectangleRec(j->heroi.bala.pos, RED);
}

void AtiraBalas(Jogo *j){
    for(int k = 0; k < j->rodada.navesPorLinha; k++){
            for(int i = j->rodada.linhasDeNaves-1; i >= 0; i--){
                if(i != j->rodada.linhasDeNaves-1 && j->nave[i+1][k].ativa)
                    break;
                
                //Atira a bala se pronta e passar na probabilidade
                if(j->nave[i][k].ativa && !j->nave[i][k].bala.ativa && GetTime()-j->nave[i][k].bala.tempo > 1){
                    
                    //Calcula se o tiro será disparado de acordo com a probabilidade da rodada
                    if(GetRandomValue(0,100) > j->nave[i][k].chanceDeTiro){
                        j->nave[i][k].bala.tempo = GetTime();
                        break;
                    }
                    
                    j->nave[i][k].bala.pos = (Rectangle){j->nave[i][k].pos.x+j->nave[i][k].pos.width/2, j->nave[i][k].pos.y+j->nave[i][k].pos.height/2, 
                    LARGURA_BALA, ALTURA_BALA};
                    j->nave[i][k].bala.ativa = 1;
                    //j->nave[i][k].bala.tempo = GetTime();
                    PlaySound(j->naves.tiro);
                }
                else if(ColisaoBalas(j, i, k)){
                    j->nave[i][k].bala.ativa = 0;
                    j->nave[i][k].bala.pos = (Rectangle){-33, -976};
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
    }else if(CheckCollisionRecs(j->nave[0][j->rodada.navesPorLinha-1].pos, j->bordas[3].pos)){
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
    for (int b = 0; b < QTD_BARREIRAS; b++) {
        if (j->barreiras[b].ativa && CheckCollisionRecs(j->nave[i][k].bala.pos, j->barreiras[b].pos) && j->nave[i][k].bala.ativa) {
            j->nave[i][k].bala.ativa = 0;
            j->nave[i][k].bala.pos.x = 999;
            j->nave[i][k].bala.pos.y = 999;
            j->barreiras[b].vida--;
            j->barreiras[b].cor.a -= j->decrementoCorBarreira;

            if (j->barreiras[b].vida == 0) {
                j->barreiras[b].ativa = 0;
            }
        }
    }
    return 0;
}

int ColisaoBalasHeroi(Jogo *j){
    // Colisao bala com as naves
    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
            if(CheckCollisionRecs(j->nave[i][k].pos, j->heroi.bala.pos) && j->nave[i][k].ativa){
                j->nave[i][k].ativa = 0;
                j->rodada.qtdNave--;
                j->pontuacao += 10;
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

    for (int b = 0; b < QTD_BARREIRAS; b++) {
        if (j->barreiras[b].ativa && CheckCollisionRecs(j->heroi.bala.pos, j->barreiras[b].pos) && j->heroi.bala.ativa) {
            j->heroi.bala.ativa = 0;
            j->heroi.bala.pos.x = 999;
            j->heroi.bala.pos.y = 999;
            j->barreiras[b].vida--;

            if (j->barreiras[b].vida == 0) {
                j->barreiras[b].ativa = 0;
            }
        }
    }
    return 0;
}

void AtualizaBarreiras(Jogo *j) {

    for (int b = 0; b < 3; b++) {
        if (j->barreiras[b].ativa) {
            j->barreiras[b].pos.x += j->barreiras[b].direcao;
            if (j->barreiras[b].pos.x <= 0 || j->barreiras[b].pos.x + j->barreiras[b].pos.width >= LARGURA_JANELA) {
                j->barreiras[b].direcao = -j->barreiras[b].direcao;
            }
        }
    }
    
}

void DesenhaVidas(Jogo *j) {
    char textoVidas[20];
    sprintf(textoVidas, "V: %d", j->heroi.vidas);
    DrawText(textoVidas, 20, 20, 20, LIGHTGRAY);
}

void DesenhaTempo(Jogo *j){
    char textoTempo[20];
    sprintf(textoTempo, "%d", (int)j->rodada.maxTempoRodada - (int)(GetTime() - j->rodada.inicioRodada));
    DrawText(textoTempo, LARGURA_JANELA-40, 20, 20, LIGHTGRAY);
}

void DesenhaPontuacao(Jogo* j){
    char textoPontuacao[30];
    sprintf(textoPontuacao, "%010d", j->pontuacao);
    DrawText(textoPontuacao, LARGURA_JANELA/2.5 - 50, 20, 20, LIGHTGRAY);
}

void DesenhaRodada(Jogo* j){
    char textoRodada[30];
    sprintf(textoRodada, "RODADA: %d", j->rodada.numeroRodadas);
    DrawText(textoRodada, LARGURA_JANELA/1.8, 20, 20, LIGHTGRAY);
}

void DesenhaBarreiras(Jogo *j) {
    for(int i = 0;i<QTD_BARREIRAS;i++) {
        if(j->barreiras[i].ativa) DrawRectangleRec(j->barreiras[i].pos, j->barreiras[i].cor);
    }
}

void IniciaBarreiras(Jogo *j, int navesPorLinha) {
    float posY[] = { ALTURA_JANELA / 2 + 225, ALTURA_JANELA / 2 + 200, ALTURA_JANELA / 2 + 175 };
    int larguraBarreira = 75;
    int vidaBase = 2;
    int incrementoVida = navesPorLinha;
    int vidaFinal = vidaBase + incrementoVida;
    j->decrementoCorBarreira = 255/(vidaFinal+5);

    for (int i = 0; i < QTD_BARREIRAS; i++) {
        float posX = GetRandomValue(10 + larguraBarreira / 2, LARGURA_JANELA - larguraBarreira / 2);
        j->barreiras[i].cor = BROWN;

        for (int k = 0; k < i; k++) {
            while (abs(j->barreiras[k].pos.x - posX) < 100) {
                posX = GetRandomValue(10 + larguraBarreira / 2, LARGURA_JANELA - larguraBarreira - 15);
            }
        }

        j->barreiras[i].pos = (Rectangle) {posX - larguraBarreira / 2, posY[i], larguraBarreira, 15};
        j->barreiras[i].ativa = 1;
        int modificadorVida = GetRandomValue(-1, 2);
        j->barreiras[i].vida = vidaFinal + modificadorVida;
        j->barreiras[i].direcao = (GetRandomValue(0, 1) == 0) ? 1 : -1;
        
    }
}

int DoubleEquals(double a, double b) {
    return fabs(a - b) < EPSILON;
}

void SalvarRanking(int pontuacao, const char *nomeJogador) {
    FILE *arquivo = fopen("ranking.txt", "r+");

    if (arquivo == NULL) {
        arquivo = fopen("ranking.txt", "w");
        if (arquivo != NULL) {
            fprintf(arquivo, "%d|%s\n", pontuacao, nomeJogador);
            fclose(arquivo);
        }
    } else {
        int pontuacoes[10];
        char nomes[10][50];
        int totalPontuacoes = 0;

        while (fscanf(arquivo, "%d|%49s", &pontuacoes[totalPontuacoes], nomes[totalPontuacoes]) == 2) {
            totalPontuacoes++;
        }
        fclose(arquivo);

        if (totalPontuacoes < 10) {
            pontuacoes[totalPontuacoes] = pontuacao;
            strcpy(nomes[totalPontuacoes], nomeJogador);
            totalPontuacoes++;
        } else if (pontuacao > pontuacoes[9]) {
            pontuacoes[9] = pontuacao;
            strcpy(nomes[9], nomeJogador);
        }

        for (int i = 0; i < totalPontuacoes; i++) {
            for (int j = i + 1; j < totalPontuacoes; j++) {
                if (pontuacoes[i] < pontuacoes[j]) {
                    int temp = pontuacoes[i];
                    pontuacoes[i] = pontuacoes[j];
                    pontuacoes[j] = temp;
                    char tempNome[50];
                    strcpy(tempNome, nomes[i]);
                    strcpy(nomes[i], nomes[j]);
                    strcpy(nomes[j], tempNome);
                }
            }
        }

        arquivo = fopen("ranking.txt", "w");
        if (arquivo != NULL) {
            for (int i = 0; i < (totalPontuacoes < 10 ? totalPontuacoes : 10); i++) {
                fprintf(arquivo, "%d|%s\n", pontuacoes[i], nomes[i]);
            }
            fclose(arquivo);
        }
    }
}

int VerificaTop(int pontuacao) {
    FILE *arquivo = fopen("ranking.txt", "r");
    if (arquivo == NULL) return 1;
    int pontuacoes[10];
    char nomes[10][50];
    for (int i = 0; i < 10; i++) {
        if (fscanf(arquivo, "%d|%49s", &pontuacoes[i], nomes[i]) != 2) break;
    }
    fclose(arquivo);
    for (int i = 0; i < 10; i++) {
        if (pontuacao > pontuacoes[i]) return 1;
    }
    return 0;
}

void solicitarNome(char *nomeJogador) {
    int tamanhoNome = 0;
    nomeJogador[0] = '\0';
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Parabéns! Você entrou no top 10! Digite seu nome:", 100, 100, 20, DARKBLUE);
        DrawText(nomeJogador, 100, 150, 20, DARKGREEN);
        int key = GetKeyPressed();
        while (key > 0) {
            if (key == KEY_BACKSPACE && tamanhoNome > 0) {
                nomeJogador[tamanhoNome - 1] = '\0';
                tamanhoNome--;
            } else if (key == KEY_ENTER && tamanhoNome > 0) {
                return;
            } else if ((key >= 32 && key <= 126) && tamanhoNome < 49) {
                nomeJogador[tamanhoNome] = (char)key;
                tamanhoNome++;
                nomeJogador[tamanhoNome] = '\0';
            }
            key = GetKeyPressed();
        }
        EndDrawing();
    }
}
