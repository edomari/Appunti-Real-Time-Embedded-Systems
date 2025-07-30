# Appunti Real-Time Embedded Systems
*Appunti per il corso di Real-Time Embedded Systems, allineati alle slide del corso.*

---

## **Scheduling in sistemi real-time**
*Fondamenti e tecniche per la gestione di processi e thread in sistemi embedded*

1. [**Processes and Threads**](/1.%20Processes%20and%20Threads.md#processi)  
   *Concetti base: processi, thread e gestione della memoria*

2. [**Concurrency**](/2.%20Concurrency.md#concorrenza)  
   *Problemi di concorrenza, race condition e sincronizzazione*

3. [**Pthreads**](/3.%20Pthreads.md#pthread)  
   *API Pthreads per la programmazione multi-thread in C*

### **Esercizi d'Esame**
*Raccolta esercizi d'esame di concorrenza e sincronizzazione*

| Esercizio | Anno | Link |
|-----------|------|------|
| 🍰 **La pasticceria** | 2006-08-06 | [📝 Vai all'esercizio](/exercices/concurrency/2006-08-06/pasticceria.md#la-pasticceria---862006) |
| 🏃 **La corsa** | 2006-14-12 | [📝 Vai all'esercizio](/exercices/concurrency/2006-14-12/corsa.md#la-corsa--14122006) |
| 🛠️ **Ufficio Riparazioni** | 2019-21-02 | [📝 Vai all'esercizio](/exercices/concurrency/2019-21-02/riparazioni.md#ufficio-riparazioni---21-febbraio-2019) |
| 🚩 **Le bandierine** | 2022-12-01 | [📝 Vai all'esercizio](/exercices/concurrency/2022-12-01/bandierine.md#le-bandierine-12012022) |
| 🎧 **La discoteca** | 2022-13-07 | [📝 Vai all'esercizio](/exercices/concurrency/2022-13-07/discoteca.md#la-discoteca---13072022) |
| 💪 **La palestra** | 2023-14-04 | [📝 Vai all'esercizio](/exercices/concurrency/2023-14-04/palestra.md#la-palestra---14042023) |
| ⚓ **Il porto** | 2023-27-06 | [📝 Vai all'esercizio](/exercices/concurrency/2023-27-06/porto.md#il-porto---27062023) |
| 🚗 **La rotonda dell'UVA** | 2024-19-04 | [📝 Vai all'esercizio](/exercices/concurrency/2024-19-04/rotonda.md#la-rotonda-delluva---19042024) |
| 🌉 **La passerella** | 2025-15-04 | [📝 Vai all'esercizio](/exercices/concurrency/2025-15-04/passerella.md#la-passerella---15042025) |
| 🚽 **Il bagno** | 2025-01-07 | [📝 Vai all'esercizio](/exercices/concurrency/2025-01-07/bagno.md#il-bagno---01072025) |
---

## **Sistemi di Tempo Reale**
*Teoria e algoritmi per lo scheduling in sistemi real-time*

4. [**Introduction to real-time scheduling** ](/4.%20Introduction%20to%20real-time%20scheduling.md#introduction-to-real-time-scheduling)  
   *Definizioni e caratteristiche dei sistemi real-time*

5. [**General Scheduling**](/5.%20General%20Scheduling.md#general-scheduling)  
   *Algoritmi di scheduling generici*

6. [**Scheduling of periodic tasks**](/6.%20Scheduling%20of%20periodic%20tasks.md#scheduling-of-periodic-tasks)  
   *Gestione di task periodici: Rate Monotonic, Deadline Monotonic, ecc*

7. [**Resource access protocols**](/7.%20Resource%20access%20protocols.md#resource-access-protocols)  
   *Protocolli per l'accesso alle risorse: PIP, PCP, HLP, ecc*

8. [**Handling hybrid task sets**](/8.%20Handling%20hybrid%20task%20sets.md#handling-hybrid-task-sets)  
   *Scheduling di task set misti (periodici + aperiodici)*

### [Esercizi real-time scheduling](/exercices/scheduling/scheduling.md)

---

## **Progetto d'Esame: Real-Time Data Center Security Monitor**

Il progetto d'esame consiste nello sviluppo di un sistema embedded implementato su Raspberry Pi Pico con FreeRTOS e micro-ROS.  
Il mio progetto integra sensori di temperatura/umidità (DHT11), rilevamento movimento (PIR), pulsante e LED RGB per la gestione della presenza di un tecnico.  
I dati raccolti vengono pubblicati tramite ROS2 su topic dedicati e visualizzati in tempo reale su una dashboard web interattiva.

**Approfondimento e dettagli:**  
➡️ [Vai alla documentazione e codice del progetto Datacenter](datacenter/README.md)

**Aspetti chiave trattati:**
- Progettazione di task concorrenti e comunicazione tramite code in FreeRTOS
- Gestione di eventi real-time e sincronizzazione tra sensori e attuatori
- Pubblicazione dati e stato su topic ROS 2
- Integrazione con una dashboard web per la visualizzazione live dei dati e degli eventi
- Esempio pratico di architettura embedded distribuita per applicazioni di sicurezza

---

