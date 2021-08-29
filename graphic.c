#include <allegro.h>
#include <math.h>
#include <stdio.h>
#include "ptask.h"
#include <string.h>
//#include <gsl/gsl_matrix.h>
//#include <gsl/gsl_blas.h>
#include "matrices.h"

#define W_win 640 //larghezza della finestra
#define H_win 480 //altezza della finestra 
#define X1 120 //posizioni di riferimento delle BITMAP che compongono la parte
#define X2 520 // grafica
#define Y1 300
#define BKG 0
#define CR_CMP_R 160//colori della bitmap contenente il robot
#define CR_CMP_G 82
#define CR_CMP_B 45
#define CR_All_R 105
#define CR_All_G 105
#define CR_All_B 105
#define scale 2//fattore di scala

//valori di OFFSET bitmap MQ 
#define W_mq 16 //dimensioni di ogni cella della matrice Q
#define H_mq 5
#define X_OFF 37 //offset delle celle della matrice q
#define Y_OFF 50

#define X_TEXT 35//valori di offset della stringa "matrice Q"
#define Y_TEXT 5

#define X_TEXT_data 15//valori di offset per il plot dei 
#define Y_TEXT_data 50//dati di interesse(parametri di learning,...)
#define FB 10//offset di "a capo" qunado stampo i dati

//elementi per aggiornamento della tabella degli stati
#define N_ST_SV 5
#define X_Mat_S_OFF 370
#define Y_Mat_S_OFF  25
#define Y_Lab_S_OFF 5
#define X_Lab_S_OFF 405
#define L_S_rect 20
#define C_S_rect 10
#define C_S_RAD 10
//elementi per l'aggiornamento del grafico 
#define G_X_OFF 50
#define G_Y_OFF 165
#define Len_Ax_X 301
#define Len_Ax_Y 141
#define X_MAX_R_L 20
#define Y_MAX_R_L 30
#define X_MIN_R_L 20
#define Y_MIN_R_L 160
#define X_EPOCH_L 300
#define Y_EPOCH_L 170
#define X_G_NAME 100
#define Y_G_NAME 10

//elementi da togliere perchè nella define di qlearning
#define N_state 49
#define N_state_x_ang 7
#define N_action 4

//parametri per il disegno del crawler più le dimensioni fisiche
#define h_floor 50
#define w_centre 150
#define d_body 0.2
#define h_body 0.03
#define r_wheel 0.015
#define r_joint 0.0075

extern float* get_state();

//inserisco altre define che identificano la forma dei parametri da cercare 
struct DOF{
    float q1;
    float q2;
    float q3;
    float q4;
    float q5;
    float q6;
};

//struct DOF state;
state joint_var;


static int figure[12];


static BITMAP * CR;
static BITMAP * MQ;
static BITMAP * P_data;
static BITMAP* GRP_STAT;
static float MQ_[N_state][N_action];
int conv_col(int col,int cscale)
{
    if(col*cscale>=255)
    {
        return(255);
    }
    else
    {
        return(col*cscale);
    }
}
void   init_s()
{
    char s[20];
    int col,VB,VR,VG,c_scale=1,i,j;
    //inizializzazione finestra allegro 
    allegro_init();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,W_win*scale,H_win*scale,0,0);
    clear_to_color(screen,BKG);
    CR=create_bitmap((X2-X1)*scale,Y1*scale);
    VB=255;
    VR=255;//valore da settare
    VG=conv_col(VR,c_scale);
    col=makecol(VR,VG,VB);
    clear_to_color(CR,makecol(255,255,255));
    line(CR,0,(CR->h-h_floor*scale),(CR->w),(CR->h-h_floor*scale),1);
    blit(CR,screen,0,0,X1*scale,0,CR->w,CR->h);
    MQ=create_bitmap(X1*scale,Y1*scale);
    clear_to_color(MQ,makecol(255,255,255));
    blit(MQ,screen,0,0,0,0,MQ->w,MQ->h);
    P_data=create_bitmap((W_win-X2)*scale,Y1*scale);
    clear_to_color(P_data,makecol(255,255,255));
    blit(P_data,screen,0,0,X2*scale,0,P_data->w,P_data->h);
    
    GRP_STAT=create_bitmap((W_win-X1)*scale,(Y1)*scale);
    printf("%d\n",W_win);
    clear_to_color(GRP_STAT,makecol(255,255,255));
    blit(GRP_STAT,screen,0,0,X1*scale,Y1*scale,GRP_STAT->w,GRP_STAT->h);
    //inizializzazione vettore ciclico S_loop
    /*for(i=0;i<N_ST_SV;i++)
    {
        Stat_lp[i]=-1;
    }*/
    // parte di inizializzazione pre-extern variable
    joint_var.q1=0;
    joint_var.q2=0;
    joint_var.q3=0;
    joint_var.q4=3.14/2;
    joint_var.q5=3*3.14/2;
    joint_var.q6=3*3.14/2;
    for(i=0;i<N_state;i++)
    {
        for(j=0;j<N_action;j++)
        {

            MQ_[i][j]=-4*i+j;
            //printf("%d ",4*i+j);
        }
        //printf("\n");
    }
    


}

//funzione che converte i metri in pixel tale che un metroo sono 1000 pixel con scala

void update_data(BITMAP* BM_TXT,
                float alpha,
                float gam,
                float eps,
                float decay,
                float dis,
                int epoch ){
    char str[25];
/*
    static float alpha;		// learning rate    
    static float gam;		// discount factor
    static float epsilon; 	// actual exploration probability
    static float decay;		// decay rate for epsilon
    */
    sprintf(str,">Learning Rate:%.4f",alpha);
    textout_ex(BM_TXT, font, str, X_TEXT_data*scale, Y_TEXT_data*scale, makecol(0,0,0), makecol(255,255,255));
    sprintf(str,">Discount Factor:%.4f",gam);
    textout_ex(BM_TXT, font, str, X_TEXT_data*scale, (Y_TEXT_data+2*FB)*scale, makecol(0,0,0), makecol(255,255,255));
    textout_ex(BM_TXT, font,">Actual Exploration", X_TEXT_data*scale, (Y_TEXT_data+3*FB)*scale, makecol(0,0,0), makecol(255,255,255));
    sprintf(str,"Probability:%.4f",eps);
    textout_ex(BM_TXT, font, str, X_TEXT_data*scale, (Y_TEXT_data+4*FB)*scale, makecol(0,0,0), makecol(255,255,255));
    textout_ex(BM_TXT, font,">Decay Rate for", X_TEXT_data*scale, (Y_TEXT_data+5*FB)*scale, makecol(0,0,0), makecol(255,255,255));
    sprintf(str," Epsilon:%.4f",decay);
    textout_ex(BM_TXT, font, str,X_TEXT_data*scale, (Y_TEXT_data+6*FB)*scale, makecol(0,0,0), makecol(255,255,255));
    sprintf(str,">Distance:%.4f",dis);
    textout_ex(BM_TXT, font, str, X_TEXT_data*scale, (Y_TEXT_data+7*FB)*scale, makecol(0,0,0), makecol(255,255,255));
    sprintf(str,">Epoch:%d",epoch);
    textout_ex(BM_TXT, font, str, X_TEXT_data*scale,  (Y_TEXT_data+8*FB), makecol(0,0,0), makecol(255,255,255));
/*
sprintf(str,">Deadline task1:%d",dl1);
    textout_ex(P_data, font, str, X_TEXT_data*scale,  (Y_TEXT_data+9*FB), makecol(
0,0,0), makecol(255,255,255));
sprintf(str,">Deadline task2:%d",dl2);
    textout_ex(P_data, font, str, X_TEXT_data*scale,  (Y_TEXT_data+10*FB), makecol(
0,0,0), makecol(255,255,255));
sprintf(str,">Deadline task1:%d",dl3);
    textout_ex(P_data, font, str, X_TEXT_data*scale,  (Y_TEXT_data+11*FB), makecol(
0,0,0), makecol(255,255,255));
sprintf(str,">Deadline task1:%d",dl4);
    textout_ex(P_data, font, str, X_TEXT_data*scale,  (Y_TEXT_data+12*FB), makecol(
0,0,0), makecol(255,255,255));

*/

blit(BM_TXT,screen,0,0,X2*scale,0,P_data->w,P_data->h);
}


void update_STAT(BITMAP* BM_SG,int new_state)
{
    static int Sl_count=0;
    static int Stat_lp[N_ST_SV];
    int i,j,k,col;
    Stat_lp[Sl_count]=new_state;
    Sl_count=(Sl_count+1)%N_ST_SV;
    textout_ex(GRP_STAT, font, "State Matrix", X_Lab_S_OFF*scale, Y_Lab_S_OFF*scale, makecol(0,0,0), makecol(255,255,255));
    for(i=0;i<N_state_x_ang;i++)
    {
        for(j=0;j<N_state_x_ang;j++)
        {
            rect(GRP_STAT,(X_Mat_S_OFF+i*L_S_rect)*scale,
            (Y_Mat_S_OFF+j*L_S_rect)*scale,
            (X_Mat_S_OFF+(i+1)*L_S_rect)*scale,
            (Y_Mat_S_OFF+(j+1)*L_S_rect)*scale,
            makecol(0,0,0));
            for(k=0;k<N_ST_SV;k++)
            {
                //printf("Lo stato %d vale %d mentre lo stato attuale vale %d\n",k,Stat_lp[k],i*N_state_x_ang+j);
                if(Stat_lp[k]==(i*N_state_x_ang+j))
                {
                    col=245*k/N_ST_SV;
                    circlefill(GRP_STAT,
                    (X_Mat_S_OFF+i*L_S_rect+C_S_rect)*scale,
                    (Y_Mat_S_OFF+j*L_S_rect+C_S_rect)*scale,
                    C_S_RAD,
                    makecol(col,col,col)
                    );
                }
            }
            //printf("%d ",4*i+j);
        }
        //printf("\n");
    }
    
}

void update_graph(BITMAP* BM_GS,float reward,int min_range,int max_range)
{
    static float  Reward_p[Len_Ax_X*scale];
    static int Rew_count=0;
    static int Rew_begin=0;
    int i,cont_plot;
    float val;
    char s[10];

    line(BM_GS,G_X_OFF*scale,G_Y_OFF*scale,G_X_OFF*scale,(G_Y_OFF-Len_Ax_Y)*scale,makecol(0,0,0));
    line(BM_GS,G_X_OFF*scale,G_Y_OFF*scale,(G_X_OFF+Len_Ax_X)*scale,G_Y_OFF*scale,makecol(0,0,0));
    sprintf(s,"%d",max_range);
    textout_ex(BM_GS, font, s, X_MAX_R_L*scale, Y_MAX_R_L*scale, makecol(0,0,0), makecol(255,255,255));
    sprintf(s,"%d",min_range);
    textout_ex(BM_GS, font, s, X_MIN_R_L*scale, Y_MIN_R_L*scale, makecol(0,0,0), makecol(255,255,255));

    textout_ex(BM_GS, font, "Epoch", X_EPOCH_L*scale, Y_EPOCH_L*scale, makecol(0,0,0), makecol(255,255,255));
    textout_ex(BM_GS, font, "Reward Plot", X_G_NAME*scale, Y_G_NAME*scale, makecol(0,0,0), makecol(255,255,255));
    if(Rew_count<Len_Ax_X*scale-1)
    {
        Reward_p[Rew_count]=reward;
        Rew_count++;
    }
    else
    {
        Rew_begin=(Rew_begin+1)%(Len_Ax_X*scale);
        Reward_p[(Rew_begin+Len_Ax_X*scale)%(Len_Ax_X*scale)]=reward;
    }
    for(i=0;i<Rew_count;i++)
    {
        cont_plot=(i+Rew_begin)%(Len_Ax_X*scale);
        if(Reward_p[cont_plot]>=max_range)
            val=(float)max_range;
        else if(Reward_p[cont_plot]<=min_range)
            val=(float)min_range;
        else
            val=Reward_p[cont_plot];
        
        val=((val-min_range)/(max_range-min_range))*(Len_Ax_Y*scale-1);
        putpixel(BM_GS,(G_X_OFF*scale+1+i),(G_Y_OFF*scale-floor(val)),makecol(255,0,0));
        
    }

}

void update_GRP_STAT(BITMAP* BM_GS,int state,float reward,int max_r,int min_r,int flag)
{
    if(flag==1)
    {
        clear_to_color(BM_GS,makecol(255,255,255));
        update_STAT(BM_GS,state);
        update_graph(BM_GS,reward,min_r,max_r);
        blit(BM_GS,screen,0,0,X1*scale,Y1*scale,BM_GS->w,BM_GS->h);
    }
}
//funzione per il calcolo dei PX per disegnare il corpo, a seguire c'è
//quella per il link 1 e poi per il secondo
int MToPx(double val,int xy)
{
    if (xy==0)
    {
        return(((int)floor(val*1000)+w_centre)*scale);
    }
    if(xy==1)
    {
        return(CR->h-((int)floor(val*1000)+h_floor)*scale);
    }
    else{
        
        return((int)floor(val*1000*scale));
    }
}
void  body_kin(int position[],state s)
{   
    
    double rot_body[4];
    double pos_body[2];
    
    rot_body[0]=cos(s.q3);
    rot_body[1]=-sin(s.q3);
    pos_body[0]=-0.1+0.1*cos(s.q3)-0.03*sin(s.q3);
    rot_body[2]=sin(s.q3);
    rot_body[3]=cos(s.q3);
    pos_body[1]=0.015+0.03*cos(s.q3)+0.1*sin(s.q3);
    position[0]=MToPx(rot_body[0]*(-d_body/2)+rot_body[1]*(-h_body/2)+pos_body[0],0);
    position[1]=MToPx(rot_body[2]*(-d_body/2)+rot_body[3]*(-h_body/2)+pos_body[1],1);
    position[2]=MToPx(rot_body[0]*(-d_body/2)+rot_body[1]*(h_body/2)+pos_body[0],0);
    position[3]=MToPx(rot_body[2]*(-d_body/2)+rot_body[3]*(h_body/2)+pos_body[1],1);
    position[4]=MToPx(rot_body[0]*(d_body/2)+rot_body[1]*(h_body/2)+pos_body[0],0);
    position[5]=MToPx(rot_body[2]*(d_body/2)+rot_body[3]*(h_body/2)+pos_body[1],1);
    position[6]=MToPx(rot_body[0]*(d_body/2)+rot_body[1]*(-h_body/2-2*r_wheel)+pos_body[0],0);
    position[7]=MToPx(rot_body[2]*(d_body/2)+rot_body[3]*(-h_body/2-2*r_wheel)+pos_body[1],1);
    position[8]=MToPx(rot_body[0]*(d_body/2-r_wheel)+rot_body[1]*(-h_body/2)+pos_body[0],0);
    position[9]=MToPx(rot_body[2]*(d_body/2-r_wheel)+rot_body[3]*(-h_body/2)+pos_body[1],1);
    position[10]=MToPx(rot_body[0]*(-d_body/2)+rot_body[1]*(-h_body/2-r_wheel)+pos_body[0],0);
    position[11]=MToPx(rot_body[2]*(-d_body/2)+rot_body[3]*(-h_body/2-r_wheel)+pos_body[1],1);
    }
void L1_kin(int position[],state s)
{
    //recupero la posizione del primo giunto direttamente dal vettore prec. calcolato
    
    position[0]=position[4];
    position[1]=position[5];

    position[2]=  MToPx(cos(s.q3)*(0.1 + 0.06 *cos(s.q4)) + 
    (-0.1 + 0.1* cos(s.q3) - 0.03*sin(s.q3)) - 
   sin(s.q3)* (0.015 + 0.06 *sin(s.q4)),0);
    position[3]= MToPx( (0.015 + 0.03 *cos(s.q3) + 0.1* sin(s.q3)) + (0.1 + 
      0.06* cos(s.q4))* sin(s.q3) + cos(s.q3) *(0.015 + 0.06* sin(s.q4)),1);
      
  
}
void L2_kin(int position[],state s)
{
    //recupero la posizione del primo giunto direttamente dal vettore prec. calcolato
    
    position[0]=position[2];
    position[1]=position[3];

    position[2]=  MToPx(
      (-0.1  + 0.1* cos(s.q3) - 0.03* sin(s.q3)) + 
   cos(s.q3) *(0.1 + 0.06* cos(s.q5) *sin(s.q4) + 
      cos(s.q4) *(0.06 + 0.06 *sin(s.q5))) - 
   sin(s.q3)* (0.015 - 0.06 *cos(s.q4)* cos(s.q5) + 
      sin(s.q4) *(0.06 + 0.06 *sin(s.q5)))
        ,0);
    position[3]= MToPx( 
     (0.015 + 0.03* cos(s.q3) + 0.1* sin(s.q3)) + 
   sin(s.q3)* (0.1 + 0.06* cos(s.q5)* sin(s.q4) + 
      cos(s.q4)* (0.06 + 0.06 *sin(s.q5))) + 
   cos(s.q3)* (0.015 - 0.06 *cos(s.q4)* cos(s.q5) + 
      sin(s.q4)* (0.06 + 0.06 *sin(s.q5)))
      ,1);
     
}

void init_CR(BITMAP*BM_CR)
{
    BM_CR=create_bitmap((X2-X1)*scale,Y1*scale);
    clear_to_color(BM_CR,makecol(255,255,255));
    line(BM_CR,0,(BM_CR->h-h_floor*scale),(CR->w),(CR->h-h_floor*scale),1);
    blit(CR,screen,0,0,X1*scale,0,CR->w,CR->h);
}
void update_CR(BITMAP* BM_CR,state joint_v)
{   
    body_kin(figure,joint_v);
    polygon(CR,5,figure,makecol(CR_CMP_R,CR_CMP_G,CR_CMP_B));
    circlefill(CR,figure[10],figure[11],MToPx(r_wheel,2),makecol(CR_All_R,CR_All_G,CR_All_B));
    circlefill(CR,figure[4],figure[5],MToPx(r_joint,2),makecol(CR_All_R,CR_All_G,CR_All_B));
   L1_kin(figure,joint_v);
    line(CR,figure[0],figure[1],figure[2],figure[3],makecol(CR_CMP_R,CR_CMP_G,CR_CMP_B));
    circlefill(CR,figure[2],figure[3],MToPx(r_joint,2),makecol(CR_All_R,CR_All_G,CR_All_B));
    L2_kin(figure,joint_v);
    line(CR,figure[0],figure[1],figure[2],figure[3],makecol(CR_CMP_R,CR_CMP_G,CR_CMP_B));
    
    blit(CR,screen,0,0,X1*scale,0,CR->w,CR->h);

}
/*
*Funzione che aggiorna la BITMAP contenente la Matrice Q, che dovrà essere 
*passsata come argomento della funzione sotto forma di puntatore;
*è possibile variare la gradazione dei color 
*/
void init_MQ(BITMAP* BM_MQ)
{
    BM_MQ=create_bitmap(X1*scale,Y1*scale);
    clear_to_color(BM_MQ,makecol(255,255,255));
    blit(BM_MQ,screen,0,0,0,0,BM_MQ->w,BM_MQ->h);
}
void update_MQ(BITMAP* BM_MQ,float * matrix,float step)
{
    int i,j,val;
    
    textout_ex(BM_MQ, font, "Matrice Q", X_TEXT*scale, Y_TEXT*scale, makecol(0,0,0), makecol(255,255,255));
    for(i=0;i<N_state;i++)
    {
        for(j=0;j<N_action;j++)
        {
            if(matrix[i*N_action+j]>0)
            {
                val=(int)floor(matrix[i*N_action+j]/step);
                if(val>255)
                    val=255;
                printf("la cella %d,%d ha %d\n",i,j,val);
                rectfill(BM_MQ,
                scale*((W_mq)*j+X_OFF),
                scale*(H_mq*i+Y_OFF),
                scale*(W_mq*(j+1)+X_OFF),
                scale*(H_mq*(i+1)+Y_OFF),
                makecol(255-val,255-val,255));
            }
            if(matrix[i*N_action+j]<0)
            {
                val=(int)floor(-matrix[i*N_action+j]/step);
                if(val>255)
                    val=255;
                printf("la cella %d,%d ha %d\n",i,j,val);
                rectfill(BM_MQ,
                scale*(W_mq)*j+X_OFF,
                scale*H_mq*i+Y_OFF,
                scale*W_mq*(j+1)+X_OFF,
                scale*H_mq*(i+1)+Y_OFF,
                makecol(255,255-val,255-val));
            }
        }
    }
    blit(BM_MQ,screen,0,0,0,0,BM_MQ->w,BM_MQ->h);
}

/*
void *update_graphic(void *arg)
{
    /*
    *
    *
    int ti;
    ti = pt_get_index(arg);
    pt_set_activation(ti);
    while (1)
    {
        /*
        * sezione con le get per ogni struttura utilizzata in 
        *mutua esclusione :
        *stato,reward,variabili di learning, MAX_MIN struct,
        *Matrice Q 
        */
        /*
        *Chiamata per ogni sottofunzione a cui sono
        *passati i valori delle struct prese 
        *
        if (pt_deadline_miss(ti))
            // <do action>;
        pt_wait_for_period(ti);
    } 
}*/

// main esistente solo per il testing 

int main()
{
    int a;
    init_s();
    update_CR(CR,joint_var);
    update_MQ(MQ,(float*)&MQ_,0.5);
    update_data(P_data,0.5,0,20.2,0,0,0);
   // update_GRP_STAT(int state,float reward,int max_r,int min_r,int flag)
    update_GRP_STAT(GRP_STAT,10,10,200,0,1);
    update_GRP_STAT(GRP_STAT,11,11,100,0,1);
    update_GRP_STAT(GRP_STAT,12,12,100,0,1);
    update_GRP_STAT(GRP_STAT,13,13,100,0,1);
    update_GRP_STAT(GRP_STAT,14,14,100,0,1);
    update_GRP_STAT(GRP_STAT,15,15,100,0,1);
    update_GRP_STAT(GRP_STAT,16,16,100,0,1);
    update_GRP_STAT(GRP_STAT,17,17,100,0,1);
    update_GRP_STAT(GRP_STAT,18,18,100,0,1);
    update_GRP_STAT(GRP_STAT,19,19,200,-200,1);

    do{
        scanf("%d",&a);
    }while(a==0); 
    printf("sono passato \n");
    allegro_exit();
    return(0);


}


