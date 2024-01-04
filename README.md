# Projet Packer Windows x86/x86_64
## Description

Le projet Packer Windows x86/x86_64 est un utilitaire et librairie de packing simple et modulable pour les exécutables Windows.  
Il permet de compresser un fichier exécutable en utilisant différents algorithmes de compression.  
L'objectif principal est de créer un exécutable minimal, comprenant uniquement les éléments nécessaires pour l'exécution.  

## Utilisation

Pour compiler le projet :  
```sh
  $ git clone https://github.com/0xC-M0NK3Y/Packer-Windows
  $ cd Packer-Windows
  $ make
```

Ensuite pour utiliser le packer :  
Pour x86_64
```sh
  $ ./bin/x86_64/packer.exe <chemin de l'exe a packer> <algorihtm de compression> <chemin de l'exe de sortie>
```
Pour x86
```sh
  $ ./bin/x86/packer.exe <chemin de l'exe a packer> <algorihtm de compression> <chemin de l'exe de sortie>
```

L'exécutable résultant sera généré dans le chemin de sortie specifié utilisant l'algorithm de compression spécifié.  

### Algorithmes disponibles

- xor
- TODO: d'autre algorithmes

## Details
  
L'executable generé ne contiendra uniquement le strict necessaire.  
Il contient trois sections : 
  - .text avec le code
  - .rdata contenant l'exécutable de base compressé
  - .idata contenant la table des imports
  
Le code de l'exécutable dans .text inclut un bootloader qui appelle la fonction de décompression, décompresse l'exécutable original depuis la section .rdata, puis appel le chargeur d'executable qui l'execute.  

## Librairie

Le projet propose également une librairie (32 ou 64 bits) avec deux fonctions définies dans le fichier d'en-tête include/packer.h :
```c
    int packer_pack_executable(char *executable, char *algorithm, char *out);
    // Cette fonction prend le chemin de l'exécutable à packer, l'algorithme de compression à utiliser, et le chemin de sortie de l'exécutable packé.
    // renvoi 0 ou un code d'erreur

    char *packer_get_error(int error);
    // Cette fonction renvoie une chaîne de caractères décrivant un code d'erreur.
```
Le .a de la librairie se trouve après compilation dans le dossier lib (dans les sous dossiers x86_64 pour la version 64 et x86 pour la version 32).  
## Personnalisation

Le projet est conçu pour être facilement modifiable.  
Il est possible d'ajouter de nouveaux algorithmes de compression dans le dossier src/algorithms, il suffit ensuite simplement de preciser ses imports (voir xor.c pour exemple), et de modifier dans src/packer.c rajouter le check pour l'algortihme utilisé.  
Il est aussi possible de modifier le bootloader, ou de changer le chargeur d'exécutable qui est actuelement très minimaliste.  
Cependant, il est déconseillé d'utiliser la libc dans l'algorithme de compression, le bootloader ou le chargeur, car elle ne sera pas initialisée.

## TODOs

- Passer argc argv depuis le bootloader ou loader avec GetCommandLineA et  
passer l'env depuis le bootloader ou loader avec GetEnvironmentStrings  
meme si cela est faisable directement depuis l'executable packé
- implementer d'autres algorithmes de compression
- Eventuellement implementer la possibilité d'utiliser une dll de compression, en ajoutant tout le code de la dll dans l'executable packé
