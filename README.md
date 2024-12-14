# Calculette pour simuler un prêt
## ✨ Permet au choix de :
- 📊 Calculer un **taux**
- ⏳ Calculer une **durée**
- 💰 Calculer une **mensualité de remboursement**
- 💵 Trouver le **capital**

Édite également un **tableau d'amortissement**.

## Installation : Fonctionne sous linux avec gtkmm-4 qu'il faudra installer en faisant :
### Pour ubuntu/debian
```
sudo apt update
sudo apt upgrade
sudo apt install libgtkmm-4.0-dev
```

### Vérifiez que la bibliothèque est installée :
```
pkg-config --modversion gtkmm-4.0
```
#### Si tout est correctement installé, cette commande renverra la version de gtkmm-4, par exemple : 4.6.0.

### Compiler un projet avec gtkmm-4
```
g++ -std=c++17 -o calculator main.cpp `pkg-config gtkmm-4.0 --cflags --libs` -Wall -Wextra -O2
```
## Non testé sous windows mais il suffit d'installer gtkmm-4 et de compiler le projet...
Téléchargement de GTK et GTKMM

GTKMM repose sur GTK, donc vous devez d'abord installer les bibliothèques GTK.

GTK4 Précompilé pour Windows : Téléchargez la version Windows de GTK à partir du site officiel :

[GTK](https://gtk.org/)

GTKMM Précompilé : Téléchargez les fichiers binaires de GTKMM. Vous pouvez les obtenir via un gestionnaire de paquets comme [MSYS2](https://www.msys2.org/) (recommandé) 

ou en compilant à partir des sources.
## Lien vers la documentation gtkmm-4
[Documentation Overview](https://gtkmm.gnome.org/en/documentation.html)

[Programming with gtkmm 4](https://gnome.pages.gitlab.gnome.org/gtkmm-documentation/)

[Download](https://gtkmm.gnome.org/fr/download.html)
