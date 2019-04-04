//Acest proiect este destinat recunoasterii de pattern-uri pentru imagini in format BitMap, citindu-le ca fisiere binare.
//Are cea mai mare precizie in recunoasterea cifrelor scrise de mana comparandu-le cu sabloane ale unor cifre scrise de tipar
//Formatul BitMap acceptat de acest program este unul simplist, tratand doar imaginile ce contin in scrierea binara cei 54 de octeti de header si tabloul de pixeli.
//Datele de intrare vor fi dupa cum urmeaza:
//                      calea fisierului de intrare;
//                      numarul de sabloane(maxim 10);
//                      caile sabloanelor.
//Datele vor fi citite de la tastatura.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//structura ce va contine octetii red, green, blue ai fiecarui pixel
typedef struct pixel
{
    unsigned char r,g,b;
};
//structura ce ne va ajuta sa retinem o detectie a unui sablon in imaginea mare
typedef struct detectii
{
    //gradul de similitudine ditre sablon si fereastra gasita (numit Cross-Correlation)
    double cc;
    //coordonatele ferestrei in imaginea mare si numarul sablonului
    unsigned long int istsus,jstsus,idrjos,jdrjos,cifra;
};

//functie pentru citirea Header-ului imaginii
//functia primeste un pointer catre un fisier BitMap si, prin intermediul celorlalti parametri, va transmite headerul alocat dinamic, dar si dimensiunile imaginii dupa cum reies din acesta.
void citesc_header(FILE * f,unsigned char **header, unsigned long int *lung, unsigned long int *lat)
{
    unsigned char h;
    int i;
    //dimensiunile pozei
    unsigned long int lungime, latime;
    fseek(f,0,SEEK_SET);

    //alocam dinamic headerul
    (*header)=(unsigned char *)malloc(55*sizeof(unsigned char));
    for(i=0; i<18; i++)
    {
        fread(&h, 1, 1, f);
        (*header)[i]=h;
    }

    (*header)[54]='\0';
    //urmatorii 8 octeti reprezinta dimensiunile imaginii, prin urmare ii citim separat
    fread(&latime,4,1,f);
    fread(&lungime,4,1,f);

    //reveire pentru a adauga toti octetii in header (inclusiv dimensiunile)
    fseek(f,-8,SEEK_CUR);

    for(i=18; i<54; i++)
    {
        fread(&h, 1, 1, f);
        (*header)[i]=h;
    }

    //actualizam dimensiunile
    (*lung)=lungime;
    (*lat)=latime;

}
//functie ce aloca dinamic o matrice de pixeli
void matrice_pixeli_neliniarizata(struct pixel ***pix, unsigned long int lungime, unsigned long int latime )
{
    (*pix)=(struct pixel **)malloc(sizeof(struct pixel*)*lungime);
    int i;
    for(i=0; i<lungime; i++)
        (*pix)[i]=(struct pixel *)malloc(latime*sizeof(struct pixel));

}
//functie ce citeste un tablou de pixeli si il converteste la o marice
void citire_pixel(FILE *fin, struct pixel **img, unsigned long int lungime, unsigned long int latime)
{
    // sarim peste header
    fseek(fin,0,SEEK_SET);
    fseek(fin, 54, SEEK_SET);
    unsigned char r,g,b,x;
    int i,j;
    //citire
    for(i=0; i<lungime; i++)
    {

        for(j=0; j<latime; j++)
        {

            fread(&b,1,1,fin);
            fread(&g,1,1,fin);
            fread(&r,1,1,fin);
            img[i][j].b=b;
            img[i][j].g=g;
            img[i][j].r=r;
        }
        //luam in considerare paddingul
        if(latime%4!=0)
            for(j=1; j<=4-(latime*3)%4; j++)
            {
                fread(&x, 1,1,fin);
            }

    }

    fclose(fin);
}
//afisarea unei imagini stocata ca matrice de pixeli
void afisare(char *calefisier,unsigned char *header, struct pixel **img, unsigned long int lungime, unsigned long int latime)
{
    FILE *fout=fopen(calefisier, "wb");
    int i,j;
    unsigned char x=0;

    //afisare header
    for(i=0; i<54; i++)
    {
        fwrite(&header[i], 1,1,fout);

    }
    //afisare tablou
    for(i=0; i<lungime; i++)
    {


        for(j=0; j<latime; j++)
        {

            fwrite(&img[i][j].b, 1,1,fout);
            fwrite(&img[i][j].g, 1,1,fout);
            fwrite(&img[i][j].r, 1,1,fout);
        }
        if(latime%4!=0)
            for(j=1; j<=4-(latime*3)%4; j++)
            {
                fwrite(&x, 1,1,fout);
            }
    }
    fclose(fout);

}
//functie ce converteste o imagine data la greyscale(alb-negru)
void greyscale(struct pixel **img, unsigned long int lungime, unsigned long int latime)
{
    int i,j;
    unsigned int g;
    unsigned char f;
    //toti pixelii vor fi modificati
    for(i=0; i<lungime; i++)
        for(j=0; j<latime; j++)
        {
            //se foloseste  formula standard
            g= 0.299*img[i][j].r+0.587*img[i][j].g+0.144*img[i][j].b;
            if(g>=255) f=255;
            else f=g;
            img[i][j].r=f;
            img[i][j].g=f;
            img[i][j].b=f;

        }
}
//aceasta este o functie ce, pentru un sablon dat, calculeaza deviatia standard si
double DeviatiaStandardSablon(struct pixel **sablon, unsigned long int luSablon, unsigned long int laSablon, double *Sb)
{
    unsigned long int nSablon=0,i,j;
    nSablon=luSablon*laSablon;
    double Sbar=0,DevStandardS=0;
    //medie pixeli
    for(i=0; i<luSablon; i++)
        for(j=0; j<laSablon; j++)

        {
            unsigned int c;
            c=(unsigned int)sablon[i][j].r;
            Sbar=Sbar+c;
        }
    Sbar=(double)Sbar/nSablon;
    //deviatia standard S

    for(i=0; i<luSablon; i++)
        for(j=0; j<laSablon; j++)
        {
            unsigned int c;
            double produs;
            c=(unsigned int)sablon[i][j].r;
            produs=(c-Sbar)*(c-Sbar);
            DevStandardS=DevStandardS+(double)produs/(nSablon-1);
        }
    DevStandardS=sqrt(DevStandardS);
    (*Sb)=Sbar;
    return DevStandardS;
}
//functie ce primeste o imagine, colturile stanga sus si dreapta jos ale unei ferestre in aceasta, o culoare si efectueaza colorarea unei rame in jurul acesteia
void coloreaza(struct pixel **imagine, unsigned long int Iinceput, unsigned long int Isfarsit, unsigned long int Jinceput, unsigned long int Jsfarsit,struct pixel culoare)
{
    int i,j;
    //conturul vertical
    for(i=Iinceput; i<=Isfarsit; i++)
    {
        imagine[i][Jinceput].r=culoare.r;
        imagine[i][Jinceput].g=culoare.g;
        imagine[i][Jinceput].b=culoare.b;
        imagine[i][Jsfarsit].r=culoare.r;
        imagine[i][Jsfarsit].g=culoare.g;
        imagine[i][Jsfarsit].b=culoare.b;

    }
    //conturul orizontal
    for(j=Jinceput; j<=Jsfarsit; j++)
    {
        imagine[Iinceput][j].r=culoare.r;
        imagine[Iinceput][j].g=culoare.g;
        imagine[Iinceput][j].b=culoare.b;
        imagine[Isfarsit][j].r=culoare.r;
        imagine[Isfarsit][j].g=culoare.g;
        imagine[Isfarsit][j].b=culoare.b;
    }

}

//functie care primeste o imagine, un sablon si vector de detectii si adauga la acesta fiecare detectie a sablonului dat, cu un Cross-Corelation mai mare de 0.5
void MatchSablon(char *calefisier,struct pixel **img, unsigned long int lungime, unsigned long int latime,unsigned int nrSablon, struct detectii **d,unsigned long int *nDetect)
{
    unsigned int i,j,k1,k2;
    unsigned long int luSablon,laSablon,nSablon,nDet=(*nDetect);

    //deschid sablonul
     FILE *f0=fopen(calefisier,"rb");
    if(f0==NULL)
    {
        printf("Nu s-a gasit fisierul");
        exit(0);
    }

    //stochez sablonul
    struct pixel **cifra0;
    unsigned char *header;
    double Sbar=0;
    //citesc sablon
    citesc_header(f0,&header,&luSablon,&laSablon);
    matrice_pixeli_neliniarizata(&cifra0,luSablon,laSablon);
    citire_pixel(f0,cifra0,luSablon,laSablon);

    //sablonul devine greyscale
    greyscale(cifra0,luSablon,laSablon);
    //dimensiune sablon
    nSablon=luSablon*laSablon;
    double DevStandardS=0;
    //deviatia standard a sablonului
    DevStandardS=DeviatiaStandardSablon(cifra0,luSablon,laSablon,&Sbar);

    //mai departe, se calculeaza Cross-Correlation dupa algoritmul standar (vezi https://en.wikipedia.org/wiki/Cross-correlation)
    for(k1=0; k1<lungime-luSablon; k1++)
        for(k2=0; k2<latime-laSablon; k2++)
        {
            double produs, fIbar=0,DevStandardI=0, DevSTandardTotal=0;
            for(i=k1; i<k1+luSablon; i++)
                for(j=k2; j<k2+laSablon; j++)
                {
                    unsigned int c;
                    c=(unsigned int)img[i][j].r;
                    fIbar=fIbar+c;
                }
            fIbar=(double)fIbar/nSablon;
            //printf("%lf ", fIbar);
            for(i=k1; i<k1+luSablon; i++)
                for(j=k2; j<k2+laSablon; j++)
                {
                    unsigned int c;
                    c=(unsigned int)img[i][j].r;
                    produs=(c-fIbar)*(c-fIbar);
                    DevStandardI=DevStandardI+produs;
                }
            DevStandardI=sqrt((double)DevStandardI/(nSablon-1));
            for(i=k1; i<k1+luSablon; i++)
                for(j=k2; j<k2+laSablon; j++)
                {
                    unsigned int c1,c2;
                    c1=(unsigned int)img[i][j].r;
                    c2=(unsigned int)cifra0[i-k1][j-k2].r;
                    produs=(c1-fIbar)*(c2-Sbar);
                    produs=(double)produs/DevStandardI;
                    produs=(double)produs/DevStandardS;
                    DevSTandardTotal=DevSTandardTotal+produs;
                }
            DevSTandardTotal=(double) DevSTandardTotal/nSablon;
            //printf("%lf ", DevSTandardTotal);
            if(DevSTandardTotal>=0.5)
            {
                struct detectii aux;
                nDet++;
                (*nDetect)=nDet;
                (*d)=(struct detectii *)realloc((*d),nDet*sizeof(struct detectii));
                aux.istsus=k1;
                aux.jstsus=k2;
                aux.idrjos=k1+luSablon;
                aux.jdrjos=k2+laSablon;
                aux.cc=DevSTandardTotal;
                aux.cifra=nrSablon;
                (*d)[nDet-1]=aux;
                //coloreaza(imgnoua,k1,k1+luSablon,k2,k2+laSablon,culoare);
                //printf("%f ", aux.cc);

            }

        }
    free(header);
    free(cifra0);
    fclose(f0);
}
//functie comparator ce va fi folosita mai departe pentru sortarea detectiilor in functie de campul cc
int comparator(const void *p,const void *q)
{
    double l=((struct detectii*)p)->cc;
    double r=((struct detectii*)q)->cc;
    return (-1)*(l*100-r*100);
}
//fubctie ce verifica gradul de suprapunere a oua ferestre
double SuprapunFerestre(struct detectii x, struct detectii y)
{
    //ariile celor 2 si a intersectiei
    double Ax,Ay,Axcapy=0, Axcupy=0;
    Ay=(y.idrjos-y.istsus)*(y.jdrjos-y.jstsus);
    Ax=Ay;
    double stanga, dreapta, sus, jos;
    //colturile suprapunerii
    if(x.jstsus>y.jstsus) stanga=x.jstsus;
    else stanga=y.jstsus;
    if(x.jdrjos>y.jdrjos) dreapta=y.jdrjos;
    else dreapta=x.jdrjos;
    if(x.idrjos<y.idrjos) jos=x.idrjos;
    else jos=y.idrjos;
    if(x.istsus>y.istsus) sus=x.istsus;
    else sus=y.istsus;

    //daca exista suprapunere
    if(stanga<dreapta && jos>sus)
    {
        Axcapy=(dreapta-stanga)*(jos-sus);
        Axcupy=Axcapy/(double)(Ax+Ay-Axcapy);
        //printf("%lf ",Axcupy);
        return Axcupy;
    }
    //daca nu exista, returnez -1
    return -1;

}
//functie de eliminare a non-maximele: primeste un vector sortat conform campului cc; dintre un grup de ferestre ce se vor suprapune, se patreaza doar cele cu cc cel mai mare
void EliminNonMaximele(struct detectii *det, unsigned long int *nr )
{
    unsigned long int i,j,nrDetectii=(*nr),k=0;
    //setam un prag de suprapunere
    double prag=0.2;
    for(i=0; i<nrDetectii-1; i++)
    {
        //daca fereastra curenta nu "a fost" streasta
        if(det[i].cc!=-1)
            //comparam cu toate celelalte
            for(j=i+1; j<nrDetectii; j++)
            {
                //daca fereastra de comparatie nu a fost stearsa
                if(det[j].cc!=-1)
                    //daca suprapunerea este suficient de mare
                    if(SuprapunFerestre(det[i],det[j])>=prag)

                      det[j].cc=-1;

            }
            k++;
    }
    //stergem detectiile marcate cu "-1"
    for(i=0; i<nrDetectii; i++)
    {
        if(det[i].cc==-1)
        {
            for(j=i; j<nrDetectii-1; j++)
                det[j]=det[j+1];
            nrDetectii--;
            i--;
        }

    }
    (*nr)=nrDetectii;

}
//functie ce primeste imaginea de pixeli, o copie a sa, dimensiunile sale si efectueaza operatia de template matching pentru un ste de sabloane
void Template_Matching(struct pixel **img, struct pixel **imgnoua, unsigned long int lungime, unsigned long int latime)
{
    struct detectii *det;
    det=NULL;
    unsigned long int nrDetectii=0;
    //identific cifrele
    //citesc nr de sabloane
    printf("Introduceti numarul de sabloane pe care doriti sa le folositi (insa nu mai mult de 10) :");
    unsigned int nrSabloane=3,i;
    scanf("%u",&nrSabloane);

    char *calefisier=(char *)malloc(220*sizeof( char));
    fgetc(stdin);
    for(i=0; i<nrSabloane; i++)
        //citesc calea sablonului
    {   printf("Introduceti calea sablonului %u:", i);
        //scanf("%s", calefisier);
        fgets(calefisier, 220, stdin);
        calefisier[strlen(calefisier)-1]='\0';
        printf("Va rugam asteptati...\n");
        //efectuez operatia de template-matching, retinand detectiile
        MatchSablon(calefisier,img,lungime,latime,i,&det,&nrDetectii);
    }
    //sortam descrescaor
    qsort(det, nrDetectii,sizeof(struct detectii),comparator);

    //eliminam non-maximele
    if(nrDetectii!=0)
    EliminNonMaximele(det,&nrDetectii);
   // printf("\n%lu \n", nrDetectii);

    //initializarea culorilor pentru fiecare sablon in parte
    struct pixel *culori=(struct pixel *)malloc(10*sizeof(struct pixel));
    culori[0].b=0;
    culori[0].g=0;
    culori[0].r=255;

    culori[1].b=0;
    culori[1].g=255;
    culori[1].r=255;

    culori[2].b=0;
    culori[2].g=255;
    culori[2].r=0;

    culori[3].b=255;
    culori[3].g=255;
    culori[3].r=0;

    culori[4].b=255;
    culori[4].g=0;
    culori[4].r=255;

    culori[5].b=255;
    culori[5].g=0;
    culori[5].r=0;

    culori[6].b=192;
    culori[6].g=192;
    culori[6].r=192;

    culori[7].b=0;
    culori[7].g=140;
    culori[7].r=255;

    culori[8].b=128;
    culori[8].g=0;
    culori[8].r=128;

    culori[9].b=0;
    culori[9].g=0;
    culori[9].r=128;
    //efectuam colorarea conform detectiilor ramase
    for(i=0; i<nrDetectii; i++)
        coloreaza(imgnoua,det[i].istsus,det[i].idrjos, det[i].jstsus,det[i].jdrjos,culori[det[i].cifra]);
    free(det);
    free(culori);
}
int main()
{
    FILE * fimagine;
    char *calefisier=(char *)malloc(220*sizeof( char));
    //citim calea fisierului pe care vom efectua operatia de template-matching
    printf("Calea fisierului pe care doriti identificarea sabloanelor: ");
    fgets (calefisier,220, stdin);
    calefisier[strlen(calefisier)-1]='\0';
    fimagine=fopen(calefisier, "rb");

    //citirea efectiva a imaginii
    struct pixel **img, **imgnoua;
    unsigned char *header;
    unsigned long int lungime, latime,i,j;

    //citire header
    citesc_header(fimagine,&header,&lungime,&latime);
    //alocam matricea imaginii
    matrice_pixeli_neliniarizata(&img,lungime,latime);
    //alocam copia
    matrice_pixeli_neliniarizata(&imgnoua, lungime, latime);
    //citim imaginea
    citire_pixel(fimagine, img, lungime, latime);
    //copiem imaginea
    for(i=0; i<lungime; i++)
        for(j=0; j<latime; j++)
            imgnoua[i][j]=img[i][j];
            //trecem in greyscale
    greyscale(img, lungime, latime);
    //operatia de template-mtching
    Template_Matching(img,imgnoua,lungime,latime);
    //afisam imaginea peste imaginea de intrare
    afisare(calefisier,header,imgnoua, lungime, latime);
    free(img);
    free(header);
    fclose(fimagine);
    return 0;

}
