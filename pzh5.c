#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h> // for pipe()
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <wait.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>

struct uzenet{
	long mtype;
	char mtext[1024];
};

int kuld(int uzenetsor, char uzi[1024]){
	struct uzenet uz = { 1 };
	//mert nem lehet tombot csak ugy lemasolni
	strncpy(uz.mtext, uzi, sizeof(uz.mtext) - 1);
	
	int status;
					//hova(uzenetsorba kuldom), mit, hossza, nulla
	status = msgsnd(uzenetsor, &uz, strlen(uz.mtext)+1, 0);
	if(status < 0){
		perror("msgsnd");
	}
	return 0;
}

int fogad(int uzenetsor, pid_t p){
	struct uzenet uz;
	int status;
					//honnan, mit, hossza, nulla, uzenet azonositoszama
	status = msgrcv(uzenetsor, &uz, 1024, 0, 0);
	if(status < 0){
		perror("msgsnd");
	}
	else{
		printf( "A kapott uzenet pid-ja: %d, szovege:  %s\n", p, uz.mtext );
	}
	return 0;
}

int main(int argc, char *argv[]){
	
	
	int uzenetsor;
	int status;
	key_t kulcs;
	
	//msgget-nek szuksege van kulcsra, amit az ftok() general
	//parancsnevvel és az 1 verzioszammal
	kulcs = ftok(argv[0], 1);
	//kulcs = ftok("/kd", 55);
	//printf ("A kulcs: %d\n", kulcs);
				//msgget(key_t key, int msgflg)
	uzenetsor = msgget(kulcs, 0600 | IPC_CREAT);
	if(uzenetsor < 0){
		perror("msgget");
		exit(EXIT_FAILURE);
	}
	
	//3--------------------------------------------------------------
	int oszt_mem_id;
    char *s;
    // osztott memoria letrehozasa, irasra olvasasra, 500 bajt mrettel
    oszt_mem_id = shmget(kulcs, 500, IPC_CREAT|S_IRUSR|S_IWUSR);//3. param ki ferhet hozza ki irhat/olvashat ki nem
    // csatlakozunk az osztott memoriahoz, 
    // a 2. parameter akkor kell, ha sajat cimhez akarjuk illeszteni
    // a 3. paraméter lehet SHM_RDONLY, ekkor csak read van   
    s = shmat(oszt_mem_id, NULL, 0);
    //mindenki aki a fork() utan letrejon mindenki ugyan azon az s cimen fogja elerni az osztottmem-et
	
	//4-----------------------------------------------------------
	int maria_to_boldi[2];
	if(pipe(maria_to_boldi) == -1){
		("Hiba a pipe nyitasakor!\n");
		exit(EXIT_FAILURE);
	}
	
	int oszt_mem_id2;
    char *z;
    // osztott memoria letrehozasa, irasra olvasasra, 500 bajt mrettel
    oszt_mem_id2 = shmget(kulcs, 500, IPC_CREAT|S_IRUSR|S_IWUSR);//3. param ki ferhet hozza ki irhat/olvashat ki nem
    // csatlakozunk az osztott memoriahoz, 
    // a 2. parameter akkor kell, ha sajat cimhez akarjuk illeszteni
    // a 3. paraméter lehet SHM_RDONLY, ekkor csak read van   
    z = shmat(oszt_mem_id2, NULL, 0);
    //mindenki aki a fork() utan letrejon mindenki ugyan azon az s cimen fogja elerni az osztottmem-et
	
	
	
	
	
	pid_t child1 = fork();
	if(child1 < 0){
		printf("Fork hiba!\n");
		exit(EXIT_FAILURE);
	}
	
	if(child1 == 0){//GYEREK-1 MENYHART
		
		char uzi1[] = "[Menyhart] : Harom kiraly mi vagyunk.\n";
		kuld(uzenetsor, uzi1);
		
		//2-------------------------------------------------------
		sleep(1);
		char uzi_m[] = "hoztunk aranyat hat marekkal\n";
		kuld(uzenetsor, uzi_m);
		
		pid_t menyhart_pid = getpid();
		status = fogad(uzenetsor, menyhart_pid);
		
		//4---------------------------------------------------------
		status = fogad(uzenetsor, menyhart_pid);
		
	}else if(child1 > 0){//SZULO
		pid_t child2 = fork();
		if(child2 < 0){
			printf("Fork hiba!\n");
			exit(EXIT_FAILURE);
		}
		if(child2 == 0){//GYEREK-2 BOLDIZSAR
			
			char uzi2[] = "[Boldizsar] : Langos csillag allt felettunk,\n";
			kuld(uzenetsor, uzi2);
			
			//4----------------------------------------------------------
			char je[256];
			read(maria_to_boldi[0], je, sizeof(je));
			printf("[Boldizsar] : Ezt uzente Maria: %s\n", je);
			
		}else if(child2 > 0){//SZULO
			pid_t child3 = fork();
			if(child3 < 0){
				printf("Fork hiba!\n");
				exit(EXIT_FAILURE);
			}
			if(child3 == 0){//GYEREK-3 GASPAR
				
				char uzi3[] = "[Gaspar] : gyalog jottunk, mert siettunk...\n";
				kuld(uzenetsor, uzi3);
				
				//3---------------------------------------------------------
				char buffer[] = "tomjent egesz vasfazekkal\n";
				strcpy(s, buffer);
				shmdt(s);
				shmctl(oszt_mem_id, IPC_RMID, NULL);
				
				//4--------------------------------------------------------
				sleep(4);	              
				printf("Gaspar ezt olvasta az osztott memoriabol: %s", z);
				// gyerek is elengedi az osztott mem-et, lekell rola csatlakozni
				shmdt(z);
				
				
			}else if(child3 > 0){//SZULO
				//SZULO !!!
				pid_t szulo_pid = getpid();
				status = fogad(uzenetsor, szulo_pid);
				
				status = fogad(uzenetsor, szulo_pid);
				
				status = fogad(uzenetsor, szulo_pid);
				
				//2---------------------------------------------------------
				status = fogad(uzenetsor, szulo_pid);
				
				char uzi_gy_m[] = "Koszonjuk szepen!\n";
				kuld(uzenetsor, uzi_gy_m);
				
				//3-----------------------------------------------------------
				sleep(1);
				printf("Maria ezt olvasta az osztott memoriabol: %s\n", s);
				shmdt(s);
				
				//4----------------------------------------------------------
				char joejt_uzi[] = "Kedves harom kiralyok, jo ejszakat kivanok!\n";
				//Boldinak
				write(maria_to_boldi[1], joejt_uzi, sizeof(joejt_uzi));
				
				//Menyhartnak
				sleep(1);
				kuld(uzenetsor, joejt_uzi);
				
				//Gasparnak
				strcpy(z, joejt_uzi);
			    // elengedjuk az osztott memoriat
			    shmdt(z);
				shmctl(oszt_mem_id2, IPC_RMID, NULL);
				
				sleep(1);
				
				wait(NULL);
			}
		}
	}
	wait(NULL);
	return 0;
	
}