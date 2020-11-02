#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>

#include "function_annex.c"

#define CHEMIN_MAX 512


//-----------------------------------------------------------------------------
// Compute of schema of Godunov
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Input

void parameters_init(parameters *ppar,
                    int option_solexacte, int option_animation, int option_godunov, int option_rusanov,
                    double xmin, double xmax, double cfl, double tmax,
                    int m, int N,
                    char * option_equation){
    // Initialize the object pointed by ppar with the arguments

    ppar->option_solexacte = option_solexacte;
    ppar->option_animation = option_animation;
    ppar->option_godunov = option_godunov;
    ppar->option_rusanov = option_rusanov;

    ppar->xmin = xmin;
    ppar->xmax = xmax;
    ppar->m = m;
    ppar->N = N;
    ppar->cfl = cfl;
    ppar->tmax = tmax;
    give_parameters(ppar, option_equation);
    ppar->option_equation = malloc(CHEMIN_MAX);
    strcpy(ppar->option_equation, option_equation);

    ppar->dx = (xmax - xmin) / N;

    ppar->xi = malloc((N+2) * sizeof(double) * m);

    if (option_godunov){
        ppar->un = malloc((N+2) * sizeof(double) * m);
        ppar->unp1 = malloc((N+2) * sizeof(double) * m);
    }
    if (option_rusanov){
        ppar->vn = malloc((N+2) * sizeof(double) * m);
        ppar->vnp1 = malloc((N+2) * sizeof(double) * m);
    }
    if (option_solexacte)
        ppar->sol = malloc((N+2) * sizeof(double)*m);

    for (int i = 0; i < N + 2; i++){

        ppar->xi[i] = xmin + ppar->dx/2 + (i-1)*ppar->dx;

        if (option_godunov)
            ppar->pboundary_spatial(ppar->xi[i], ppar->un + i*m);

        if (option_rusanov)
            ppar->pboundary_spatial(ppar->xi[i], ppar->vn + i*m);

        if (option_solexacte)
            ppar->psolexacte(ppar->xi[i], tmax, ppar->sol + i*m);
        
    }
}

void parameters_init_file(parameters *ppar, char * path_input, char * path_output, int option_animation, int option_godunov, int option_rusanov){
    // Initialize the object pointed by ppar with the file of path name_input

    FILE * file = NULL;
    char * line = malloc(CHEMIN_MAX);
    char * str = malloc(CHEMIN_MAX);
    const char * separators = " \n";
    const char * separators1 = "/";

    //--------------------------------------------------------
    // Sauvegarde du nom du fichier
    
    char * name_file = malloc(CHEMIN_MAX);
    strcpy(name_file, path_input);
    strcpy(str, path_input);

    strtok(str, separators1); 
    while ( (str=strtok(NULL, separators1)) != NULL){
        strcpy(name_file, str);
    }

    //--------------------------------------------------------
    // Lecture du fichier

    file = fopen(path_input, "r");

    // La consigne + le saut de ligne
    fgets(line, CHEMIN_MAX, file);
    printf("Initialisation <- %s\n", path_input);
    fgets(line, CHEMIN_MAX, file);

    // option_solexacte :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    int option_solexacte = atoi(str);

    // option_equation :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    char * option_equation = malloc(CHEMIN_MAX);
    strcpy(option_equation, str);

    // xmin :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double xmin = atof(str);

    // xmax :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double xmax = atof(str);

    // cfl :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double cfl = atof(str);

    // m :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    int m = atoi(str);

    // N :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    int N = atoi(str);

    // tmax :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double tmax = atof(str);

    fclose(file);

    //--------------------------------------------------------
    // Initialization of ppar
    
    // Save of path_input
    ppar->path_input = malloc(CHEMIN_MAX);
    strcpy(ppar->path_input, path_input);
    
    // Save of name_input
    ppar->name_file = malloc(CHEMIN_MAX);
    strcpy(ppar->name_file, name_file);
    
    // Creation of path of output (ppar->complete_output_path)
    ppar->complete_path_output = malloc(CHEMIN_MAX);
    strcpy(ppar->complete_path_output, path_output);
    strcat(ppar->complete_path_output, ppar->name_file);

    // Initialization of other parameters
    parameters_init(ppar,
                    option_solexacte, option_animation, option_godunov, option_rusanov,
                    xmin, xmax, cfl, tmax,
                    m, N, option_equation);

    //--------------------------------------------------------
    // Creation of folder

    // Creation of folder output
    mkdir(ppar->complete_path_output, ACCESSPERMS);
    
    // Creation of parameters
    par_create_parameters(ppar);

    if (option_animation){
        
        if (option_godunov){
            ppar->int_tnow_gd = 0;

            // Creation of folder plots in output
            char * path_plots_godunov = malloc(CHEMIN_MAX);
            strcpy(path_plots_godunov, ppar->complete_path_output);
            strcat(path_plots_godunov, "/plots_godunov");

            mkdir(path_plots_godunov, ACCESSPERMS);

            free(path_plots_godunov);

            // Creation of plots0.dat
            par_create_plots(ppar, 0);
        }

        if (option_rusanov){
            ppar->int_tnow_ru = 0;

            // Creation of folder plots in output
            char * path_plots_rusanov = malloc(CHEMIN_MAX);
            strcpy(path_plots_rusanov, ppar->complete_path_output);
            strcat(path_plots_rusanov, "/plots_rusanov");

            mkdir(path_plots_rusanov, ACCESSPERMS);

            free(path_plots_rusanov);

            // Creation of plots0.dat
            par_create_plots(ppar, 1);
        }
    }

    printf("Creation of folfer -> %s\n", ppar->complete_path_output);

    free(name_file);
    free(line);

    printf("Fin Initilization\n");
}


//-----------------------------------------------------------------------------
// Output

void parameters_plot(parameters *ppar){
    // Create and fill the folder of output (of path out_path)
    // with the result of ppar
    
    // Creation of file plot.dat
    par_create_plot(ppar);
    
    // Creation and execution of file plotcom.gnu
    par_create_execute_gnu(ppar);
    
    // Creation of animation
    if (ppar->option_animation){
        if (ppar->option_godunov)
            par_create_animation(ppar, 0);
        if (ppar->option_godunov)
            par_create_animation(ppar, 1);
    }

    printf("Fin Plot\n");
}


//-----------------------------------------------------------------------------
// Free

void parameters_free(parameters *ppar){
    // Liberate tables the of ppar

    free(ppar->xi);
    free(ppar->un);
    free(ppar->unp1);
    free(ppar->sol);

    printf("Fin Liberation parameters\n");
}


//-----------------------------------------------------------------------------
// Solveur

void godunov_solve(parameters *ppar, int option_visual){
    // Solve the problem of ppar
    // option_visual give visuality on terminal
    
    int m = ppar->m;

    if (option_visual){
        printf("Debut Resolution godunov\n");
    }

    time_t begin = time(NULL);
    
    double tnow = 0;
    while(tnow < ppar->tmax){
        
        // calcul de la vitesse max
        double vmax = 0;
        for (int i = 0; i < ppar->N + 2; i++){
            double vloc = fabs(ppar->plambda_ma(ppar->un + m * i));
            vmax = vmax > vloc ? vmax : vloc;
        }
        
        ppar->dt = ppar->cfl * ppar->dx / vmax;
        for(int i = 1; i < ppar->N+1; i++){
            double flux[m];
            ppar->pfluxnum_gd(ppar->un + i*m, ppar->un + (i+1)*m, flux);
            for(int iv = 0; iv < m; iv++){
                ppar->unp1[i*m + iv] = ppar->un[i*m + iv] - ppar->dt/ppar->dx * flux[iv];
            }
            ppar->pfluxnum_gd(ppar->un + (i - 1) * m, ppar->un + i * m, flux);
            for(int iv = 0; iv < m;iv++){
                ppar->unp1[i * m + iv] += ppar->dt / ppar->dx * flux[iv];
            }
        }
        // mise à jour
        tnow += ppar->dt;

        if (option_visual){
            printf("tnow = %f vmax = %f tmax = %f\n", tnow, vmax, ppar->tmax);
        }
        
        // conditions aux limites
        ppar->pboundary_temporal_left(ppar->xmin, tnow, ppar->unp1);
        ppar->pboundary_temporal_right(ppar->xmax, tnow, ppar->unp1 + (ppar->N+1)*m);

        memcpy(ppar->un, ppar->unp1, (ppar->N + 2) * m *sizeof(double));

        if (ppar->option_animation){
            ppar->int_tnow_gd++;
            par_create_plots(ppar, 0);
        }
    }
    time_t end = time(NULL);

    ppar->time = (unsigned long) difftime(end, begin);

    if (option_visual){
        printf("Fin Resolution godunov\n");
    }
}

void rusanov_solve(parameters * ppar, int option_visual){
    // Solve the problem of ppar
    // option_visual give visuality on terminal
    
    int m = ppar->m;

    if (option_visual){
        printf("Debut Resolution rusanov\n");
    }

    time_t begin = time(NULL);
    
    double tnow = 0;
    while(tnow < ppar->tmax){
        
        // calcul de la vitesse max
        double vmax = 0;
        for (int i = 0; i < ppar->N + 2; i++){
            double vloc = fabs(ppar->plambda_ma(ppar->vn + m * i));
            vmax = vmax > vloc ? vmax : vloc;
        }
        
        ppar->dt = ppar->cfl * ppar->dx / vmax;
        for(int i = 1; i < ppar->N+1; i++){
            double flux[m];
            ppar->pfluxnum_ru(ppar->vn + i*m, ppar->vn + (i+1)*m, flux);
            for(int iv = 0; iv < m; iv++){
                ppar->vnp1[i*m + iv] = ppar->vn[i*m + iv] - ppar->dt/ppar->dx * flux[iv];
            }
            ppar->pfluxnum_ru(ppar->vn + (i - 1) * m, ppar->vn + i * m, flux);
            for(int iv = 0; iv < m;iv++){
                ppar->vnp1[i * m + iv] += ppar->dt / ppar->dx * flux[iv];
            }
        }
        // mise à jour
        tnow += ppar->dt;

        if (option_visual){
            printf("tnow = %f vmax = %f tmax = %f\n", tnow, vmax, ppar->tmax);
        }
        
        // conditions aux limites
        ppar->pboundary_temporal_left(ppar->xmin, tnow, ppar->vnp1);
        ppar->pboundary_temporal_right(ppar->xmax, tnow, ppar->vnp1 + (ppar->N+1)*m);

        memcpy(ppar->vn, ppar->vnp1, (ppar->N + 2) * m *sizeof(double));

        if (ppar->option_animation){
            ppar->int_tnow_gd++;
            par_create_plots(ppar, 0);
        }
    }
    time_t end = time(NULL);

    ppar->time = (unsigned long) difftime(end, begin);

    if (option_visual)
        printf("Fin Resolution rusanov\n");
}

//-----------------------------------------------------------------------------
// Calcul des erreurs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Input

void parameters_error_init(parameters_error *pparerr,
                        char * name_file,
                        int option_godunov, int option_rusanov,
                        double xmin, double xmax, double cfl, double tmax,
                        int m, int len_liste_N, int * liste_N,
                        char * option_error, char * option_equation){
    // Initialize the parameters_error with the arguments
    
    pparerr->name_file = name_file;

    pparerr->option_godunov = option_godunov;
    pparerr->option_rusanov = option_rusanov;

    pparerr->xmin = xmin;
    pparerr->xmax = xmax;
    pparerr->cfl = cfl;
    pparerr->m = m;
    pparerr->len_liste_N = len_liste_N;
    pparerr->liste_N = liste_N;
    pparerr->tmax = tmax;

    pparerr->option_error = malloc(CHEMIN_MAX);
    pparerr->option_equation = malloc(CHEMIN_MAX);
    strcpy(pparerr->option_error, option_error);
    strcpy(pparerr->option_equation, option_equation);

    give_error_parameters(pparerr, option_error);

    pparerr->liste_error = malloc(len_liste_N * sizeof(double));
    pparerr->liste_time = malloc(len_liste_N * sizeof(unsigned long));

    printf("Fin Initialisation\n");
}

void parameters_error_init_file(parameters_error *pparerr, char * name_input, int option_godunov, int option_rusanov){
    // Initialize of parameters_error pparerr with file of path name_input

    FILE * file = NULL;
    char * line = malloc(CHEMIN_MAX);
    char * str = malloc(CHEMIN_MAX);
    const char * separators = " \n";
    const char * separators1 = "/";
    
    //--------------------------------------------------------
    // Sauvegarde du nom du fichier
    
    char * name_file = malloc(CHEMIN_MAX);
    strcpy(name_file, name_input);
    strcpy(str, name_input);

    strtok(str, separators1); 
    while ( (str=strtok(NULL, separators1)) != NULL){
        strcpy(name_file, str);
    }

    //--------------------------------------------------------
    // Lecture du fichier

    file = fopen(name_input, "r");

    // La consigne + le saut de ligne
    fgets(line, CHEMIN_MAX, file);
    printf("Initialisation <- %s\n", name_input);
    fgets(line, CHEMIN_MAX, file);

    // option_error :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    char * option_error = malloc(CHEMIN_MAX);
    strcpy(option_error, str);

    // option_equation :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    char * option_equation = malloc(CHEMIN_MAX);
    strcpy(option_equation, str);

    // xmin :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double xmin = atof(str);

    // xmax :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double xmax = atof(str);

    // cfl :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double cfl = atof(str);

    // m :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    int m = atoi(str);

    // len_liste_N :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    int len_liste_N = atoi(str);

    // liste_N :
    int * liste_N = malloc(len_liste_N * sizeof(int));

    fgets(line, CHEMIN_MAX, file);
    str = strtok(line, separators);
    for (int i=0; i<len_liste_N; i++){
        str = strtok(NULL, separators);
        liste_N[i] = atoi(str);
    }

    // tmax :
    fgets(line, CHEMIN_MAX, file);

    str = strtok(line, separators);
    str = strtok(NULL, separators);
    double tmax = atof(str);

    fclose(file);

    //--------------------------------------------------------
    // Initialisation of pparerr 
    
    parameters_error_init(pparerr, name_file,
                    option_godunov, option_rusanov,
                    xmin, xmax, cfl, tmax,
                    m, len_liste_N, liste_N,
                    option_error, option_equation);

    free(option_error);
    free(option_equation);
}


//-----------------------------------------------------------------------------
// Ouput

void parameters_error_plot(parameters_error *pparerr, char * output_path){
    // Creation of folder (of path output_path) of result of pparerr

    // Creation du dossier
    char * output_path_final = malloc(CHEMIN_MAX);
    strcpy(output_path_final, output_path);
    strcat(output_path_final, pparerr->name_file);
    
    mkdir(output_path_final, ACCESSPERMS);

    // Creation du fichier parameters
    parerr_create_parameters(pparerr, output_path_final);

    // Creation du fichier plot.dat
    parerr_create_plot(pparerr, output_path_final);

    // Creation du fichier plotcom.gnu si il n'existe pas
    parerr_create_execute_gnu(pparerr, output_path_final);

    printf("Fin Plot -> %s\n", output_path_final);

    free(output_path_final);

}


//-----------------------------------------------------------------------------
// Free

void parameters_error_free(parameters_error * pgd){
    // Liberate the tables of pgd

    free(pgd->liste_N);
    free(pgd->liste_error);
    free(pgd->liste_time);

    printf("Fin Liberation\n");
}


//-----------------------------------------------------------------------------
// Calcul

void parameters_error_compute(parameters_error *pparerr){
    // Compute the error between the numeric solution and exact solution of
    // same problem with different N (give by pparerr->liste_N)

    parameters par;
    
    for (int i=0; i<pparerr->len_liste_N; i++){

        parameters_init(&par,
                        1, 0, pparerr->option_godunov, pparerr->option_rusanov,
                        pparerr->xmin, pparerr->xmax, pparerr->cfl, pparerr->tmax,
                        pparerr->m, pparerr->liste_N[i],
                        pparerr->option_equation);

        godunov_solve(&par, 0);

        pparerr->liste_error[i] = pparerr->perror((par.N+2)*par.m, par.un, par.sol);
        pparerr->liste_time[i] = par.time;

        printf("Compute error for N=%d error=%f time=%ld s\n", par.N, pparerr->liste_error[i], pparerr->liste_time[i]);   
    }

    printf("Fin calcul des erreurs\n");

}