    # Created By Nicolas Stenuit
# Reviewed By Lucas Garcia
# Version 3.0 = 21/05
#########################################
# import des packages nécessaires.
import serial
from tkinter import *
from tkinter.ttk import Progressbar
import re
import tkinter as tk
import matplotlib.pyplot as plt
import numpy as np
import scipy.fftpack
from fpdf import FPDF
from datetime import datetime
import os
import sys
import glob
import serial
#########################################
# Definition de nos variables globales
def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result[0]
COM=serial_ports()
Fe = 100
T = 1.0 / Fe #sample spacing
baudrate = 115200
datas = []
ser = serial.Serial(COM, baudrate, timeout=1)
firstclick = True
rapport = False
nbcapteur = 1 #nombre d'accéléromètres employés
######################################### OK !
def rapport_gen(nom, prenom, n_date):
    # fonction servant à générer un pdf servant de rapport pour le client.
    global rapport
    #print(rapport)
    if rapport == True:   # si l'analyse à été réalisée -> on génère le pdf
        #print("pdf généré")
        nom_ln = str("Nom : " + nom)
        prenom_ln = str("Prenom : " + prenom)
        ndate = n_date.format('%d/%m/%Y')
        ndate_ln = str("Date de naissance : " + ndate)
        auj = str("Date de mesure : " + str(datetime.now().strftime("%d/%m/%Y : %H:%M")))
        # variable servant à identifier le patient

    # Config PDF generator
        pdf = FPDF(orientation='P', unit='mm', format='A4')
        pdf.add_page()
        pdf.set_font("Arial", size=12)

    # Textes
        pdf.text(10, 10, prenom_ln)
        pdf.text(10, 15, nom_ln)
        pdf.text(10, 20, ndate_ln)
        pdf.text(130, 10, auj)

    #Images
    #Capt 1
        pdf.image('Images/Analyse-capt-1X.png', x=0, y=30, w=200)
        pdf.image('Images/Analyse-capt-1Y.png', x=0, y=110, w=200)
        pdf.image('Images/Analyse-capt-1Z.png', x=0, y=190, w=200)

        pdf.output("Rapport_" + nom + "_" + prenom + ".pdf")
        rapport = False # reset de la valeur pour analyse future
    #Open PDF
        os.startfile("Rapport_" + nom + "_" + prenom + ".pdf")
######################################### OK !
def define_value(choix):
    # fonction permettant de définir les constantes du nombres d'époque et du nombre de ligne de données attendue par le
    # microcontrolleur
    good = 0
    while good == 0:
        ser.write(choix.encode())  # envoi du choix sur le port série
        if choix == 'A':
            Nb_ligne = 30 * Fe
            Nb_ech = 15
            good = 1
        elif choix == 'B':
            Nb_ligne = 60 * Fe
            Nb_ech = 30
            good = 1
        elif choix == 'C':
            Nb_ligne = 90 * Fe
            Nb_ech = 45
            good = 1
        elif choix == 'D':
            Nb_ligne = 120 * Fe
            Nb_ech = 60
            good = 1
        elif choix == 'E':
            Nb_ligne = 150 * Fe
            Nb_ech = 75
            good = 1
        elif choix == 'F':
            Nb_ligne = 180 * Fe
            Nb_ech = 90
            good = 1
        else:
            print("Veuillez rentrer un choix valide")
    return Nb_ligne, Nb_ech
######################################### OK
def read_serial(Nb_ligne):
    #fonction permettant de lire et de selectionner l'information envoyée sur le port série par le controlleur
    for i in range(0, Nb_ligne+30*(Nb_ligne//3000)):
        line = ser.readline()    #lit la ligne envoyée sur le port série et stocke les valeurs dans un string.
        if line:
            reg = re.search(r'\'(.*?)\\', str(line)) # Regex permettant d'isoler l'information nécessaire proprement.
            data = str(reg.group(1))  # stocke l'information mis en evidence par la regex dans une variable
            data = data.split(',')    # segmente la variable en une liste dont chaque cellule correspond a un axe
            datas.append(data)       # ajoute la liste précédente a la fin d'une matrice
            print(line)
        #print(round(100 * i / Nb_ligne))  # indique le pourcentage de completion de l'analyse
        if (round(100 * i / Nb_ligne) != progress['value'] and round(100 * i / Nb_ligne)%5==0): #limite à 100 le nb de rafraichissement de la loading bar
            progress['value'] = round(100 * i / Nb_ligne)
            fenetre.update_idletasks()
    return datas

######################################### OK
def get_axis_value(Axe,data_List,Nb_ligne,):
    # fonction permettant d'isoler les données de chaque axe dans des listes séparées
    List = []
    for i in range(0,Nb_ligne):
        ech =float(data_List[i][Axe])
        List.append(ech)
    return List
######################################### MODIF
def plot(k,Nb_ligne,Data_axis,axis_split):
    # fonction permettant l'analyse fréquentielle ainsi que la génération de graphiques

    # définition de l'axe
    if k % (3*nbcapteur) == 0:
        Axe = str("X")
    elif k % (3*nbcapteur) == 1:
        Axe = str("Y")
    else:
        Axe = str("Z")

    ffts = []
    t = np.linspace(0.0, Nb_ligne * T, Nb_ligne) # definition de notre vecteur temporel allant de 0 jusqu'a la valeur
                                                 # définie par l'utilisateur
    xf = np.linspace(0.0,30,2*Fe)                 # vecteur fréquentiel
    for element in range(len(axis_split)):       # pour chaque époque -> générer une fft
        yf = scipy.fftpack.fft(axis_split[element]) # genere la fft
        ffts.append(np.abs(yf))                     # nous n'avons besoin de que la valeur absolue de la fft
    somme = [sum(row[i] for row in ffts) for i in range(len(ffts[0]))] # calcul une somme pour chaque i eme valeurs des époques
    moy =[x /len(ffts) for x in somme]                                 # division par le nombre d'époque pour avoir une moyenne
    MOY = np.asarray(moy)                                              # utilisation de numpy pour un meilleur affichage


    plt.figure(figsize=(12,4.8))     # paramètre pour générer des graphiques
    plt.subplot(211)  # subplot pour le graphique temporel
    plt.title("Évolution temporelle & fréquentielle du capteur  - Axe : " + Axe)
    plt.plot(t,Data_axis,linewidth=0.5)
    plt.ylabel('Amplitude du signal')
    plt.xlabel('Temps')

    plt.subplot(212) #subplot pour le graphique fréquentiel
    plt.plot(xf,2*MOY/len(MOY))
    plt.ylabel('Amplitude de la FFT')
    plt.xlabel('Fréquence')
    axes = plt.gca()
    axes.set_xlim([0, 25])
    plt.grid(True)
    plt.savefig('Images/Analyse-capt-' + Axe + '.png')
#########################################
def affiche():
    # fonction servant a envoyer le bon caractère sur le port série en fonction du choix des checkbox sur le GUI
    arg = var_case.get()
    if arg == 0:
        choix = "A"
    elif arg == 1:
        choix = "B"
    elif arg == 2:
        choix = "C"
    elif arg == 3:
        choix = "D"
    elif arg == 4:
        choix = "E"
    elif arg == 5:
        choix = "F"
    #print(choix)
    return choix
######################################### MODIF
def start():
    global rapport # définition d'une variable globale pour empêcher l'utilisation du bouton imprimer avant d'avoir réalisé l'analyse.
    bouton_print.configure(state="disabled")
    bouton_start.configure(state="disabled")
    choix = affiche() # fonction permettant d'envoyer le bon caractère sur le port série pour définir le temps d'acquisition du programme.
    Nb_ligne, Nb_ech = define_value(choix) # fonction permettant de définir les constantes utilisées dans le code en fonction du choix de l'utilisateur.
    Datas = read_serial(Nb_ligne) # Lecture des données envoyées par l'arduino sur le port série et enregistrement des données dans une liste.
    axis_split = [] # définition d'une liste pour les époques de 2 secondes
    for k in range(0,3*nbcapteur):    # boucle for allant de 0 jusqu'au nombre d'axe*nombre de capteurs ( 1 capteur * 3 axes)
        Data_Axis = get_axis_value(k,Datas,Nb_ligne) # fonction utilisée pour changer la taille de la matrice Datas pour récuperer les valeurs pour chaque axe
        axis_split = [Data_Axis[x:x+2*Fe] for x in range(0,len(Data_Axis),2*Fe)] # Split de la liste des valeurs pour un axe en époque de 2 secondes( 2*Fe )
        plot(k,Nb_ligne,Data_Axis,axis_split) # fonction servant à generer nos axes
    rapport = True # autorisation d'utilisation du bouton d'impression
    bouton_print.configure(state="normal")
    bouton_start.configure(state="normal")
    return axis_split
#########################################
def rap_gen():
    # fonction utilisé pour pouvoir generer un rapport pdf grace au bouton imprimer
    rapport_gen(var_nom.get(), var_prenom.get(), var_date.get())
#########################################
def on_entry_click(event):
    # fonction permettant de vider les champs de texte en appuyant sur la fenêtre
    global firstclick

    if firstclick:
        firstclick = False
        entry1.delete(0, "end")
        entry2.delete(0, "end")
        entry3.delete(0, "end")

def ifcompleted(*args):
    if ((var_nom.get() != entry1_def_msg and len(var_nom.get()) > 0) and  # nom inchangé ou vide
            (var_prenom.get() != entry2_def_msg and len(var_prenom.get()) > 0) and  # prenom inchangé ou vide
            (var_date.get() != entry3_def_msg and len(var_date.get()) > 0)):  # date inchangée ou vide
        bouton_start.configure(state="normal")
    else:
        bouton_start.configure(state="disabled")
#########################################
fenetre = Tk()
#########################################
# variable récoltant les données du patient
var_nom = StringVar()
var_prenom = StringVar()
var_date = StringVar()
var_state = "Appuyez sur démarrer pour commencer l'analyse"
fenetre.title("Tremor Measurement")
#########################################
# le code suivant permet la création de boutons, texte d'entrée et checkbox que nous pouvons placer a notre guise.
# nous avons utilisé un système de coordonées( row & column) pour placer nos éléments.
# le paramètre sticky permet de décentrer le contenu d'une case en suivant les points cardinaux ( w = Ouest = a gauche
# de sa cellule). Nous avons aussi utilisé le principe de padding permettant d'espacer les bords des cellules
# ( pad = padding extérieur) ou l'intérieur de la cellule de ses bords (ipad = internal pad )
var_case = IntVar()
cb = []

label = Label(fenetre, text="Nom du patient: ")
label.grid(row=0, column=0, sticky='w')

entry1 = Entry(fenetre, bd=1, width=33, textvariable=var_nom)
entry1_def_msg='Veuillez rentrer le nom du patient:'
entry1.insert(0, entry1_def_msg)
entry1.bind('<FocusIn>', on_entry_click)
entry1.grid(row=0, column=1, sticky='w')
var_nom.trace("w", ifcompleted)


label = Label(fenetre, text="Prénom du patient: ")
label.grid(row=1, column=0, sticky='w')

entry2 = Entry(fenetre, bd=1, width=33, textvariable=var_prenom)
entry2_def_msg='Veuillez rentrer le prénom du patient:'
entry2.insert(0, entry2_def_msg)
entry2.bind('<FocusIn>', on_entry_click)
entry2.grid(row=1, column=1, sticky='w')
var_prenom.trace("w", ifcompleted)

label = Label(fenetre, text="Date de naissance du patient:")
label.grid(row=2, column=0, sticky='w')

entry3 = Entry(fenetre, bd=1, width=33, textvariable=var_date)
entry3_def_msg='JJ/MM/AAAA'
entry3.insert(0, entry3_def_msg)
entry3.bind('<FocusIn>', on_entry_click)
entry3.grid(row=2, column=1, sticky='w')
var_date.trace("w", ifcompleted)

for i in range(6):
    cb.append(Checkbutton(fenetre, text=str((i + 1) * 30) + " Secondes", onvalue=i, variable=var_case))
    # création automatique des checkbox. pour en générer plus ou moins il suffit de changer le paramètre présent dans le
    # range.
    cb[i].grid(row=i, column=2)

bouton_start = Button(fenetre, text="Démarrer", command=start)
bouton_start.configure(state="disabled")
bouton_start.grid(row=6, column=0, sticky='e')

bouton_stop = Button(fenetre, text="Annuler", command=fenetre.quit)
bouton_stop.grid(row=6, column=1, sticky='w', padx=17)
bouton_print = Button(fenetre, text="Imprimer le rapport", command=rap_gen)
bouton_print.configure(state="disabled")
progress = Progressbar(fenetre, length = 100, mode = 'determinate')
progress.grid(row=6, column=2, sticky='w', padx=15)
bouton_print.grid(row=12, column=2, sticky='s', pady=10)
# On démarre la boucle Tkinter qui s'interompt quand on ferme la fenêtre
fenetre.mainloop()
#########################################