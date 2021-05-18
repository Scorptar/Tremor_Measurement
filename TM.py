import serial
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showerror
from threading import Thread
import requests
import re
import matplotlib
matplotlib.use("Agg")
from scipy.fft import fft, fftfreq
import matplotlib.pyplot as plt
import numpy as np
import scipy.fftpack
from fpdf import FPDF
from datetime import datetime
from math import ceil
import os
import os.path
from os import path
import sys
import glob
import serial
import time
import xlsxwriter
import pathlib

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

# Global variables

COM = serial_ports() #Check for the right COM port
Fe = 100
T = 1.0 / Fe  # sample spacing
baudrate = 1000000
firstclick = True
ser= serial.Serial(COM, baudrate, timeout=1)
nbcapteur = 4 # nombre d'accéléromètres employés
simu=False #set on True if using ADXL362_SIMULATOR.INO

class Data_Acquisition(Thread):

    def __init__(self, nom, prenom, date, duree):
        super().__init__()
        self.nom = nom
        self.prenom = prenom
        self.date = date            # birth date
        self.duree= duree           # wanted measure time window
        self.Nb_ligne= 0
        self.choix= 0
        self.datas = []


    def define_parameters(self): #define the wanted measure time window and array size

        if self.duree == 0:
            self.choix = "A"         # 30 seconds measure time window
        elif self.duree == 1:
            self.choix = "B"         # 60 seconds measure time window
        elif self.duree == 2:
            self.choix = "C"         # 90 seconds measure time window
        elif self.duree == 3:
            self.choix = "D"         #120 seconds measure time window
        elif self.duree == 4:
            self.choix = "E"         #150 seconds measure time window
        elif self.duree == 5:
            self.choix = "F"         #180 seconds measure time window
        self.Nb_ligne=30 * (self.duree + 1) * Fe #3000 for 30s, 6000 for 60s... 18000 for 180s
        return self.Nb_ligne

    def run(self):
        ser.write(self.choix.encode())  # send choice
        while(True):
            line = ser.readline()  # read and save one sample
            if line:
                reg = re.search(r'\'(.*?)\\', str(line))  # Filter the sample (REGEX)
                data = str(reg.group(1))  # Save filtered data
                data = data.split(',')  # The sample is divided by commas between each axis data
                self.datas.append(data)  # Append each sample to the data list
                print(data)
            if(len(self.datas)==self.Nb_ligne) : #if it is the end
                print("break")
                break; #stop the thread

    def get_datas(self):
        return self.datas

class Data_Processing(Thread):

    def __init__(self, nom, prenom, date, Nb_ligne, data):
        super().__init__()
        self.nom = nom
        self.prenom = prenom
        self.date = date            # birth date
        self.Nb_ligne= Nb_ligne
        self.datas = data
        self.pdf_filename=""

    def run(self):
        while(True):
            self.plot()  # generate FFT graphs for each axis of each sensor
            self.plotmoy()  # generate FFT graphs for the average of each axis from all sensors
            self.excel_gen()  # save measure data in an Excel spreadsheet
            self.printPDF()  # Print the PDF report
            break;

    def get_axis_value(self, Axe, data_List, Nb_ligne, ):
        # fonction permettant d'isoler les données de chaque axe dans des listes séparées
        List = []
        for i in range(0, Nb_ligne):
            ech = float(data_List[i][Axe])
            List.append(ech)
        return List

    def plot(self): #generate FFT graphs for each axis of each sensor
        # fonction permettant l'analyse fréquentielle ainsi que la génération de graphiques
        for k in range(0,
                       3 * nbcapteur):  # boucle for allant de 0 jusqu'au nombre d'axe*nombre de capteurs ( 1 capteur * 3 axes)
            Data_Axis = self.get_axis_value(k, self.datas,
                                       self.Nb_ligne)  # fonction utilisée pour changer la taille de la matrice Datas pour récuperer les valeurs pour chaque axe
            axis_split = [Data_Axis[x:x + 2 * Fe] for x in range(0, len(Data_Axis),2 * Fe)]  # Split de la liste des valeurs pour un axe en époque de 2 secondes( 2*Fe )
            # définition de l'axe
            if k % 3 == 0:
                if k == 0:
                    Axe = "1X"
                else:
                    Axe = str((k // 3) + 1) + str("X")
            elif k % 3 == 1:
                Axe = str(ceil(k / 3)) + str("Y")
            else:
                Axe = str(ceil(k / 3)) + str("Z")

            y = np.array(Data_Axis)
            t = np.linspace(0.0, self.Nb_ligne * T, self.Nb_ligne, endpoint=False)
            yf = fft(y)
            xf = fftfreq(self.Nb_ligne, T)[:self.Nb_ligne // 2]

            fig = plt.figure(figsize=(12, 4.8))  # paramètre pour générer des graphiques
            plt.subplot(211)  # subplot pour le graphique temporel
            plt.title("Évolution temporelle & fréquentielle du capteur  - Axe : " + Axe)
            plt.plot(t, Data_Axis, linewidth=0.5)
            plt.ylabel('Amplitude du signal [mm/s²]')
            plt.xlabel('Temps [s]')

            plt.subplot(212)  # subplot pour le graphique fréquentiel
            plt.plot(xf, 2.0 / self.Nb_ligne * np.abs(yf[0:self.Nb_ligne // 2]))
            plt.ylabel('Amplitude de la FFT [mm/s²]')
            plt.xlabel('Fréquence [Hz]')
            axes = plt.gca()
            axes.set_xlim([0, 25])
            plt.grid(True)
            fig.tight_layout()
            if not os.path.exists('Images'):
                curr_path = pathlib.Path().absolute()
                directory = "Images"
                Images_path = os.path.join(curr_path, directory)
                os.mkdir(Images_path)
            plt.savefig('Images/' + Axe + '.png')
            plt.close(fig)

    def plotmoy(self): #generate FFT graphs for the average of each axis from all sensors
        Datas_T = np.transpose(self.datas)
        x_Datas_T = Datas_T[::3]
        y_Datas_T = Datas_T[1::3]
        z_Datas_T = Datas_T[2::3]
        xmoy_Datas_T = np.mean(x_Datas_T.astype(float), axis=0)
        ymoy_Datas_T = np.mean(y_Datas_T.astype(float), axis=0)
        zmoy_Datas_T = np.mean(z_Datas_T.astype(float), axis=0)
        moy_Datas_T = np.array([xmoy_Datas_T, ymoy_Datas_T, zmoy_Datas_T])
        for i in range(0, len(moy_Datas_T)):
            axis_moy_Datas_T = moy_Datas_T[i]
            if i % 3 == 0:
                Axe = str("X")
            elif i % 3 == 1:
                Axe = str("Y")
            else:
                Axe = str("Z")

            y = np.array(axis_moy_Datas_T)
            t = np.linspace(0.0, self.Nb_ligne * T, self.Nb_ligne, endpoint=False)
            yf = fft(y)
            xf = fftfreq(self.Nb_ligne, T)[:self.Nb_ligne // 2]

            fig = plt.figure(figsize=(12, 4.8))  # paramètre pour générer des graphiques
            plt.subplot(211)  # subplot pour le graphique temporel
            plt.title("Évolution temporelle & fréquentielle moyenne  - Axe : " + Axe)
            plt.plot(t, axis_moy_Datas_T, linewidth=0.5)
            plt.ylabel('Amplitude du signal [mm/s²]')
            plt.xlabel('Temps [s]')

            plt.subplot(212)  # subplot pour le graphique fréquentiel
            plt.plot(xf, 2.0 / self.Nb_ligne * np.abs(yf[0:self.Nb_ligne // 2]))
            plt.ylabel('Amplitude de la FFT [mm/s²]')
            plt.xlabel('Fréquence [Hz]')
            axes = plt.gca()
            axes.set_xlim([0, 25])
            plt.grid(True)
            fig.tight_layout()
            if not os.path.exists('Images'):
                curr_path = pathlib.Path().absolute()
                directory = "Images"
                Images_path = os.path.join(curr_path, directory)
                os.mkdir(Images_path)
            plt.savefig('Images/Moy' + Axe + '.png')
            plt.close(fig)

    def excel_gen(self): #save measure data in an Excel spreadsheet
        # Create a workbook and add a worksheet.
        excel_name_date = str(datetime.now().strftime("%Y_%m_%d_%H_%M"))
        workbook = xlsxwriter.Workbook('Données_' + str((self.nom).upper()) + '_' + str(self.prenom) + "-" + excel_name_date + ".xlsx")
        worksheet = workbook.add_worksheet(excel_name_date)
        worksheet.set_column('A:Z', 12)  # set column width (auto adjust not possible
        # Add a (bold + border + center) format for title line
        title = workbook.add_format({'bold': True})
        title.set_border(1)
        title.set_align('center')
        # Same without bold for other cells
        regular = workbook.add_format()
        regular.set_border(1)
        # Generate title line with axis labels
        titleline = ["Temps [s]"]
        for k in range(0, 3 * nbcapteur):
            if k % 3 == 0:
                if k == 0:
                    Axe = "Capteur 1X"
                else:
                    Axe = str("Capteur ") + str((k // 3) + 1) + str("-X")
            elif k % 3 == 1:
                Axe = str("Capteur ") + str(ceil(k / 3)) + str("-Y")
            else:
                Axe = str("Capteur ") + str(ceil(k / 3)) + str("-Z")
            titleline.append(Axe)
        # Start from the first cell. Rows and columns are zero indexed.
        row = 0
        col = 0
        worksheet.write_row(row, col, titleline, title)
        # Append to data a Time stamp
        timestamp = np.linspace(0.0, (self.Nb_ligne / 100) - T, self.Nb_ligne)
        Datas = np.array(self.datas, dtype=np.float32)
        timestamp = np.column_stack([timestamp, Datas])
        # Start from the second line. Rows and columns are zero indexed.
        col = 0
        row = 1
        # Iterate over the data and write it out column by column.
        for row, data in enumerate(timestamp):
            worksheet.write_row(row + 1, col, data, regular)
        workbook.close()

    def printPDF(self): #Print the PDF report

        # Patient general data
        nom_ln = str("Nom : " + self.nom)
        prenom_ln = str("Prenom : " + self.prenom)
        birthdate = self.date.format('%d/%m/%Y')
        birthdate_ln = str("Date de naissance : " + birthdate)
        auj = str("Date de mesure : " + str(datetime.now().strftime("%d/%m/%Y : %H:%M")))

        # Configuration of PDF generator
        pdf = FPDF(orientation='P', unit='mm', format='A4')
        pdf.set_font("Arial", size=12)
        nbpage = str(nbcapteur + 1)

        for i in range(1, nbcapteur + 2):
            pdf.add_page()

            # Textes
            if i == 1 and nbcapteur > 1:
                msg = str("Analyse moyenne pour l'ensemble des capteurs")
                pdf.text(67, 25, msg)
            else:
                msg = str("Analyse du capteur " + str(i - 1))
                pdf.text(90, 28, msg)
            pdf.text(10, 10, prenom_ln)
            pdf.text(10, 15, nom_ln)
            pdf.text(10, 20, birthdate_ln)
            pdf.text(130, 10, auj)

            # Images
            if i == 1 and nbcapteur > 1:
                pdf.image('Images/MoyX.png', x=10, y=27, w=200)
                pdf.image('Images/MoyY.png', x=10, y=105, w=200)
                pdf.image('Images/MoyZ.png', x=10, y=180, w=200)
            elif i > 1 and nbcapteur > 1 :
                pdf.image('Images/' + str(i - 1) + 'X.png', x=10, y=30, w=200)
                pdf.image('Images/' + str(i - 1) + 'Y.png', x=10, y=105, w=200)
                pdf.image('Images/' + str(i - 1) + 'Z.png', x=10, y=180, w=200)
            else :
                pdf.image('Images/' + str(i) + 'X.png', x=10, y=30, w=200)
                pdf.image('Images/' + str(i) + 'Y.png', x=10, y=105, w=200)
                pdf.image('Images/' + str(i) + 'Z.png', x=10, y=180, w=200)
            # Page number
            pdf.set_y(260)
            pdf.cell(0, 10, 'Page ' + str(pdf.page_no()) + '/' + nbpage, 0, 0, 'C')

        # Generate pdf
        pdfname_date = str(datetime.now().strftime("%Y_%m_%d_%H_%M"))
        self.pdf_filename = "Rapport_" + self.nom.upper() + "_" + self.prenom + "-" + pdfname_date + ".pdf"
        pdf.output(self.pdf_filename)

        #Open pdf

        os.startfile(self.pdf_filename)

    def get_pdf_filename(self):
        return self.pdf_filename

class App(tk.Tk):
    def __init__(self):
        super().__init__()

        # Configure window settings
        self.title('Tremor Measurement')
        self.geometry('580x200')
        self.grid_propagate(False)
        self.resizable(0, 0)

        #Configure window items
        self.create_items()

    def create_items(self):

        #Var declaration
        self.var_nom = tk.StringVar()
        self.var_prenom = tk.StringVar()
        self.var_date = tk.StringVar()
        self.var_nom = tk.StringVar()
        self.var_duree = tk.IntVar()
        self.cb=[]

        #Elements configuration
        #Text zone for the last name
        self.label=tk.Label(self, text="Nom du patient : ")
        self.label.grid(row=0, column=0, sticky='w')
        self.entry1=tk.Entry(self, bd=1, width=45, textvariable=self.var_nom)
        self.entry1_def_msg='Veuillez rentrer le nom du patient:'
        self.entry1.insert(0, self.entry1_def_msg)
        self.entry1.bind('<FocusIn>', self.on_entry_click)
        self.entry1.grid(row=0, column=1, sticky='w')
        self.var_nom.trace("w", self.ifcompleted)

        # Text zone for the first name
        self.label = tk.Label(self, text="Prénom du patient : ")
        self.label.grid(row=1, column=0, sticky='w')
        self.entry2 = tk.Entry(self, bd=1, width=45, textvariable=self.var_prenom)
        self.entry2_def_msg = 'Veuillez rentrer le prénom du patient:'
        self.entry2.insert(0, self.entry2_def_msg)
        self.entry2.bind('<FocusIn>', self.on_entry_click)
        self.entry2.grid(row=1, column=1, sticky='w')
        self.var_prenom.trace("w", self.ifcompleted)

        # Text zone for the birth name
        self.label = tk.Label(self, text="Date de naissance du patient : ")
        self.label.grid(row=2, column=0, sticky='w')
        self.entry3 = tk.Entry(self, bd=1, width=45, textvariable=self.var_date)
        self.entry3_def_msg = 'JJ/MM/AAAA'
        self.entry3.insert(0, self.entry3_def_msg)
        self.entry3.bind('<FocusIn>', self.on_entry_click)
        self.entry3.grid(row=2, column=1, sticky='w')
        self.var_date.trace("w", self.ifcompleted)

        # Text zone 1 for general information
        self.info_msg = "Complétez les informations sur le patient avant de démarrer."
        self.info_label = tk.Label(self, text=self.info_msg)
        self.info_label.grid(row=4, column=0, sticky='w', padx=3, columnspan=2)

        # Text zone 2 for general information
        self.info2_label = tk.Label(self)
        self.info2_label.grid(row=5, column=0, sticky='w', padx=3, columnspan=2)

        # List of Checkbuttons to choose the wanted measure time window
        for i in range(6):
            self.cb.append(tk.Checkbutton(self, text=str((i + 1) * 30) + " Secondes", onvalue=i, variable=self.var_duree))
            self.cb[i].grid(row=i, column=2, sticky='w', padx=10)

        #Start button
        self.bouton_start = tk.Button(self, text="Démarrer")
        self.bouton_start.configure(state="disabled")
        self.bouton_start['command'] = self.start
        self.bouton_start.grid(row=6, column=0, sticky='s', pady=10, columnspan=3)

    def on_entry_click(self, event):    # Empty text zones on first click
        global firstclick
        if firstclick:
            firstclick = False
            self.entry1.delete(0, "end")
            self.entry2.delete(0, "end")
            self.entry3.delete(0, "end")

    def start(self):                    #happen when pressing start button

        self.bouton_start['state']=tk.DISABLED
        self.entry1['state'] = tk.DISABLED
        self.entry2['state'] = tk.DISABLED
        self.entry3['state'] = tk.DISABLED
        for i in range(6):
            self.cb[i]['state']= tk.DISABLED
        self.data_acquisition= Data_Acquisition(self.var_nom.get(),
                                           self.var_prenom.get(),
                                           self.var_date.get(),
                                           self.var_duree.get())
        self.lines_number= self.data_acquisition.define_parameters()  #define the wanted measure time window and array size
        self.measure_time= 44 * (self.lines_number // 3000) #for 30s of measure, 14s more to transmit
        self.data_acquisition.start() #launch data acquisition thread
        self.monitor_acquisition(self.data_acquisition,self.measure_time) #monitor data acquisition thread

    def monitor_acquisition(self,thread,countdown):
        if thread.is_alive():
            if simu :
                if countdown > self.measure_time * 30 / 33 :
                    self.info_label['text'] = "Prise de mesure en cours."
                    self.info2_label['text'] = "Temps restant : {} secondes".format(countdown)
                if countdown <= self.measure_time * 30 / 33 :
                    self.info_label['text'] = "Récupération des données en cours."
                    self.info2_label['text'] = "Temps restant : {} secondes".format(countdown)
                if countdown>0 :
                    self.after(1000, self.monitor_acquisition, self.data_acquisition, countdown - 1) #rappelle la fonction 1 seconde plus tard
            if not simu :
                if countdown > self.measure_time * 14 / 44 :
                    self.info_label['text'] = "Prise de mesure en cours."
                    self.info2_label['text'] = "Temps restant : {} secondes".format(countdown)
                if countdown <= self.measure_time * 14 / 44 :
                    self.info_label['text'] = "Récupération des données en cours."
                    self.info2_label['text'] = "Temps restant : {} secondes".format(countdown)
                if countdown>0 :
                    self.after(1000, self.monitor_acquisition, self.data_acquisition, countdown - 1) #rappelle la fonction 1 seconde plus tard
        else :
            self.info_label['text'] = "Traitement et sauvegarde des données en cours."
            self.info2_label['text'] = "Ouverture automatique du PDF dans quelques instants."
            self.datas=self.data_acquisition.get_datas() #Get datas
            self.data_processing = Data_Processing(self.var_nom.get(),
                                                     self.var_prenom.get(),
                                                     self.var_date.get(),
                                                     self.lines_number,
                                                     self.datas)
            self.data_processing.start() #launch data processing thread
            self.monitor_processing(self.data_processing) #monitor data processing thread

    def monitor_processing(self, thread):
        if thread.is_alive():
            self.info_label['text'] = "Traitement et sauvegarde des données en cours."
            self.info2_label['text'] = "Ouverture automatique du PDF dans quelques instants."
            self.after(100, self.monitor_processing, self.data_processing) # recheck thread status in 100 ms
        else :
            self.info_label['text'] = "Rapport généré et données enregistrées."
            self.info2_label['text'] = "Appuyez sur démarrer pour commencer une nouvelle analyse."
            self.bouton_start['state'] = tk.NORMAL
            self.entry1['state'] = tk.NORMAL
            self.entry2['state'] = tk.NORMAL
            self.entry3['state'] = tk.NORMAL
            for i in range(6):
                self.cb[i]['state'] = tk.NORMAL

    def ifcompleted(self, *args): #Chech that each text zone is filled before allowing to start the acquisition
        if ((self.var_nom.get() != self.entry1_def_msg and len(self.var_nom.get()) > 0) and        # nom inchangé ou vide
            (self.var_prenom.get() != self.entry2_def_msg and len(self.var_prenom.get()) > 0) and  # prenom inchangé ou vide
            (self.var_date.get() != self.entry3_def_msg and len(self.var_date.get()) > 0)):        # date inchangée ou vide
            self.bouton_start.configure(state="normal")
            self.info_label['text'] = "Appuyez sur démarrer pour commencer l'analyse."
            self.info2_label['text'] = ""
        else:
            self.bouton_start.configure(state="disabled")
            self.info_label['text'] = "Veuillez remplir tous les champs."
            self.info2_label['text'] = ""

if __name__ == "__main__":
    app = App()
    app.mainloop()