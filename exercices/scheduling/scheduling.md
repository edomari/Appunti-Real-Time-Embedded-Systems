# Esercizi real-time scheduling

### Questo task-set è schedulabile con Rate Monotonic?

|  | $C_i$ | $D_i = T_i$ |
| --- | --- | --- |
| $\tau_1$ | 2 | 6 |
| $\tau_2$ | 2 | 8 |
| $\tau_3$ | 2 | 12 |

Con il test basato su utilizzazione ho $U = \frac{2}{6} + \frac{2}{8} + \frac{2}{12} = 0,75$ e $U_{lub} = 3(\sqrt[3]{2}-1) = 0,779$ $\rightarrow$ il task è certamente schedulabile dato che $U \lt U_{lub}$.

---

|  | $C_i$ | $D_i = T_i$ |
| --- | --- | --- |
| $\tau_1$ | 3 | 5 |
| $\tau_2$ | 1 | 8 |
| $\tau_3$ | 1 | 10 |

Con il test basato su utilizzazione ho $U = \frac{3}{5} + \frac{1}{8} + \frac{1}{10} = 0,825$ e $U_{lub} = 3(\sqrt[3]{2}-1) = 0,779$ $\rightarrow$ non posso concludere se il task è schedulabile dato che $U \geq U_{lub}$.

Proseguo con l'Hyperbolic Bound $\rightarrow \prod_{i=1}^n (U_i + 1) < 2$.
- $(U_1 + 1)\cdot(U_2 + 1)\cdot(U_3 + 1)$
- $(\frac{3}{5} + 1)\cdot(\frac{1}{8} + 1)\cdot(\frac{1}{10} + 1)$
- $\frac{8}{5}\cdot\frac{9}{8}\cdot\frac{11}{10} = 1,98 \lt 2$

Il task-set è schedulabile secondo RM!

---


|  | $C_i$ | $D_i = T_i$ |
| --- | --- | --- |
| $\tau_1$ | 1 | 4 |
| $\tau_2$ | 2 | 6 |
| $\tau_3$ | 3 | 10 |

Con il test basato su utilizzazione ho $U = \frac{1}{4} + \frac{2}{6} + \frac{3}{10} = 0,88$ e $U_{lub} = 3(\sqrt[3]{2}-1) = 0,779$ $\rightarrow$ non posso concludere se il task è schedulabile dato che $U \geq U_{lub}$.

Proseguo con l'**Hyperbolic Bound** $\rightarrow \prod_{i=1}^n (U_i + 1) \lt 2$.
- $(U_1 + 1)(U_2 + 1)(U_3 + 1)$
- $(\frac{1}{4} + 1)(\frac{2}{6} + 1)(\frac{3}{10} + 1)$
- $\frac{5}{4}\cdot\frac{8}{6}\cdot\frac{13}{10} = 2,16 \not\lt 2$

Non possiamo concludere se il taskset è schedulabile o no. Applichiamo la **Response Time Analysis**.

$R_1^{(0)} = C_1 + 0 = 1 + 0 = 1\leq 4$ → perchè non c’è nessuno a priorità più alta.

$R_2^{(0)} = 2 + \lceil \frac{2}{4} \rceil 1 = 2 + 1 \cdot 1 = 3 $

$R_2^{(1)} = 2 + \lceil \frac{3}{4} \rceil 1 = 2 + 1 \cdot 1 = 3 \leq 6$

$R_3^{(0)} = 3 + \lceil \frac{3}{6} \rceil 2 + \lceil \frac{3}{4} \rceil 1 = 3 + 1 \cdot 2 + 1 \cdot 1 = 6 $

$R_3^{(1)} = 3 + \lceil \frac{6}{6} \rceil 2 + \lceil \frac{6}{4} \rceil 1 = 3 + 1 \cdot 2 + 2 \cdot 1 = 7 $

$R_3^{(2)} = 3 + \lceil \frac{7}{6} \rceil 2 + \lceil \frac{7}{4} \rceil 1 = 3 + 2 \cdot 2 + 2 \cdot 1 = 9 $

$R_3^{(3)} = 3 + \lceil \frac{9}{6} \rceil 2 + \lceil \frac{9}{4} \rceil 1 = 3 + 2 \cdot 2 + 3 \cdot 1 = 10 $

$R_3^{(4)} = 3 + \lceil \frac{10}{6} \rceil 2 + \lceil \frac{10}{4} \rceil 1 = 3 + 2 \cdot 2 + 3 \cdot 1 = 10 \leq 10 $


Il task-set è schedulabile!

---

|  | $C_i$ | $D_i = T_i$ |
| --- | --- | --- |
| $\tau_1$ | 1 | 4 |
| $\tau_2$ | 2 | 6 |
| $\tau_3$ | 3 | 8 |

Con il test basato su utilizzazione ho $U = \frac{1}{4} + \frac{2}{6} + \frac{3}{8} = 0,95$ e $U_{lub} = 3(\sqrt[3]{2}-1) = 0,779$ $\rightarrow$ non posso concludere se il task è schedulabile dato che $U \geq U_{lub}$.

Proseguo con l'Hyperbolic Bound $\rightarrow \prod_{i=1}^n (U_i + 1) < 2$.
- $(U_1 + 1)\cdot(U_2 + 1)\cdot(U_3 + 1)$
- $(\frac{1}{4} + 1)\cdot(\frac{2}{6} + 1)\cdot(\frac{3}{8} + 1)$
- $\frac{5}{4}\cdot\frac{8}{6}\cdot\frac{11}{8} = 2,29 \not\lt 2$

Non possiamo concludere se il taskset è schedulabile o no. Applichiamo la Response Time Analysis.

$R_1^{(0)} = C_1 + 0 = 1 + 0 = 1\leq 4$ → perchè non c’è nessuno a priorità più alta.

$R_2^{(0)} = 2 + \lceil \frac{2}{4} \rceil 1 = 2 + 1 \cdot 1 = 3 $

$R_2^{(1)} = 2 + \lceil \frac{3}{4} \rceil 1 = 2 + 1 \cdot 1 = 3 \leq 6$

$R_3^{(0)} = 3 + \lceil \frac{3}{6} \rceil 2 + \lceil \frac{3}{4} \rceil 1 = 3 + 1 \cdot 2 + 1 \cdot 1 = 6 $ 

$R_3^{(1)} = 3 + \lceil \frac{6}{6} \rceil 2 + \lceil \frac{6}{4} \rceil 1 = 3 + 1 \cdot 2 + 2 \cdot 1 = 7 $ 

$R_3^{(2)} = 3 + \lceil \frac{7}{6} \rceil 2 + \lceil \frac{7}{4} \rceil 1 = 3 + 2 \cdot 2 + 2 \cdot 1 = 9 \not\leq 8 $


Il task-set _**NON**_ è schedulabile con RM!

---

|  | $C_i$ | $D_i = T_i$ |
| --- | --- | --- |
| $\tau_1$ | 3 | 6 |
| $\tau_2$ | 7 | 28 |
| $\tau_3$ | 5 | 30 |

Con il test basato su utilizzazione ho $U = \frac{3}{6}+\frac{7}{28}+\frac{5}{30} = 0,916$ e $U_{lub} = 3(\sqrt[3]{2}-1) = 0,779$ $\rightarrow$ Forse si può fare perchè $U \geq U_{lub}$, sono nel margine di incertezza.

$R_1^{(0)} = C_1 + 0 = 3 \leq 6$ → perchè non c’è nessuno a priorità più alta.

$R_2^{(0)} = 7 + \lceil\frac{7}{6}\rceil 3 = 7 + 2 \cdot 3 = 13 $ 

$R_2^{(1)}= 7 + \lceil\frac{13}{6}\rceil 3 = 7 + 3 \cdot 3 = 16 $ 

$R_2^{(2)}= 7 + \lceil\frac{16}{6}\rceil 3 = 7 + 3 \cdot 3 = 16 \leq 28 $


$R_3^{(0)} = 5 + \lceil\frac{5}{6}\rceil 3+\lceil\frac{5}{28}\rceil 7 = 5 + 1 \cdot 3 + 1 \cdot 7 =15 $ 

$R_3^{(1)}= 5 + \lceil\frac{15}{6}\rceil 3+\lceil\frac{15}{28}\rceil 7 = 5 + 3 \cdot 3 + 1 \cdot 7 =21 $ 

$R_3^{(2)}= 5 + \lceil\frac{21}{6}\rceil 3+\lceil\frac{21}{28}\rceil 7 = 5 + 4 \cdot 3 + 1 \cdot 7 = 24 $ 

$R_3^{(3)}= 5 + \lceil\frac{24}{6}\rceil 3+\lceil\frac{24}{28}\rceil 7 = 5 + 4 \cdot 3 + 1 \cdot 7 = 24 \leq 30 $

Il task-set è schedulabile!

---

### Questo task-set è schedulabile con EDF?
|  | $C_i$ | $D_i = T_i$ |
| --- | --- | --- |
| $\tau_1$ | 1 | 4 |
| $\tau_2$ | 2 | 6 |
| $\tau_3$ | 3 | 8 |

Con il test basato su utilizzazione ho $U = \frac{1}{4} + \frac{2}{6} + \frac{3}{8} = 0,95 \lt 1$.

Il task-set è schedulabile con EDF!

---

|  | $C_i$ | $D_i$ | $T_i$ |
| --- | --- | --- | --- |
| $\tau_1$ | 2 | 5 | 6 |
| $\tau_2$ | 2 | 4 | 8 |
| $\tau_3$ | 4 | 8 | 12 |

Calcolo $H$ e $L^*$:

$H = lcm(6,8,12)=3\cdot2,2^3$,$2^2\cdot 3 = 3 \cdot 2^3 = 24$

$U = \frac{2}{6}+\frac{2}{8}+\frac{4}{12}=\frac{8 + 6 + 8}{24}=\frac{\not 22^{11}}{\not 24^{12}} = \frac{11}{12}$

$$L^*= \frac{(T_1-D_1)\frac{C_1}{T_1}+(T_2-D_2)\frac{C_2}{T_2}+(T_3-D_3)\frac{C_3}{T_3}}{1-U} =\\ 
\frac{(6-5)\frac{2}{6}+(8-4)\frac{2}{4}+(12-8)\frac{4}{12}}{1-\frac{11}{12}}\\ 
=\frac{\frac{\not2^1}{\not6^3}+\not4\cdot\frac{2}{\not4}+ 4\cdot\frac{\not4^1}{\not12^3}}{\frac{1}{12}} = \frac{\frac{1}{3}+2+\frac{4}{3}}{\frac{1}{12}} = \frac{\frac{2+6+8}{6}}{\frac{1}{12}} = \frac{16}{\not 6^1} \cdot \not{12}^2 = 32
$$

**Deadlines $D_i + kT_i$** $\rightarrow (4,5,8,11,12,17,20,23)$
- $i=1$ $\rightarrow 5,11,17,23$
- $i=2$ $\rightarrow 4,12,20$
- $i=3$ $\rightarrow 8,20$
$$G(L)= \sum_{i=1}^n\lfloor\frac{L + T_i - D_i}{T_i}\rfloor C_i$$

$L = 4 \rightarrow \lfloor\frac{4 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{4 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{4 + 12 - 8}{12}\rfloor 4 = 0\cdot 2 + 1 \cdot 2 + 0\cdot 4 = 2 \leq 32 
$

$L = 5 \rightarrow \lfloor\frac{5 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{5 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{5 + 12 - 8}{12}\rfloor 4 = 1\cdot 2 + 1\cdot 2 + 0\cdot 4 = 4 \leq 32 
$

$L = 8 \rightarrow \lfloor\frac{8 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{8 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{8 + 12 - 8}{12}\rfloor 4 = 1\cdot 2 + 1\cdot 2 + 1\cdot 4 = 8 \leq 32 
$

$L = 11 \rightarrow \lfloor\frac{11 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{11 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{11 + 12 - 8}{12}\rfloor 4 = 2\cdot 2 + 1\cdot 2 + 1\cdot 4 = 10 \leq 32 
$

$L = 12 \rightarrow \lfloor\frac{12 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{12 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{12 + 12 - 8}{12}\rfloor 4 = 2\cdot 2 + 2\cdot 2 + 1\cdot 4 = 12 \leq 32 
$

$L = 17 \rightarrow \lfloor\frac{17 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{17 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{17 + 12 - 8}{12}\rfloor 4 = 3\cdot 2 + 2\cdot 2 + 1\cdot 4 = 14 \leq 32 
$

$L = 20 \rightarrow \lfloor\frac{20 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{20 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{20 + 12 - 8}{12}\rfloor 4 = 3\cdot 2 + 3\cdot 2 + 2\cdot 4 = 20 \leq 32 
$

$L = 23 \rightarrow \lfloor\frac{23 + 6 - 5}{6}\rfloor 2 + \lfloor\frac{23 + 8 - 4}{8}\rfloor 2 +\lfloor\frac{23 + 12 - 8}{12}\rfloor 4 = 4\cdot 2 + 3\cdot 2 + 2\cdot 4 = 22 \leq 32 
$

Il task-set è schedulabile con EDF!

---
### Questo task-set è schedulabile con Deadline Monotonic?

|  | $C_i$ | $D_i$ | $T_i$ |
| --- | --- | --- | --- |
| $\tau_2$ | 2 | 4 | 8 |
| $\tau_1$ | 2 | 5 | 6 |
| $\tau_3$ | 4 | 8 | 12 |

$R_2^{(0)} = C_2 + 0 = 2 \lt 4$

$R_1^{(0)} = C_1 + \lceil\frac{C_1}{T_2}\rceil C_2 = 2 + \lceil \frac{2}{8}\rceil 2 = 2 + 1 \cdot 2 = 4 \lt 5$

$R_1^{(1)} = 2 + \lceil\frac{4}{8}\rceil 2 = 2 + 1 \cdot 2 = 4 \lt 5$

$R_3^{(0)} = C_3 + \lceil\frac{C_3}{T_1}\rceil C_1 + \lceil\frac{C_3}{T_2}\rceil C_2 = 4 + \lceil\frac{4}{6}\rceil 2 + \lceil\frac{4}{8}\rceil 2 = 4 + 1 \cdot 2 + 1 \cdot 2 = 8 \lt 12$

$R_3^{(1)} = 8 + \lceil\frac{8}{6}\rceil 2 + \lceil\frac{8}{8}\rceil 2 = 4 + 2 \cdot 2 + 1 \cdot 2 = 10 \not\lt 8 \rightarrow \tau_3$ missa la deadline.

---

### Bloccaggio con Priority Inheritance e con Priority Ceiling

|       | $R_1$  | $R_2$  | $R_3$  | B<sub>PIP</sub> | B<sub>PCP</sub> |
|-------|-----|-----|-----|-----------------|-----------------|
| $\tau_1$ |     | 20  |     | 5               | 5               |
| $\tau_2$ | 5   |     | 10  | 20              | 10              |
| $\tau_3$ |     | 5   | 5   | 15              | 10              |
| $\tau_4$ |     |     | 5   | 10              | 10              |
| $\tau_5$ | 10  | 3   |     |                 |                 |

In PIP, un task può essere bloccato da ogni risorsa utilizzata da task a priorità inferiore. Per ogni risorsa si considera al massimo un bloccaggio.

- **$\tau_5$**: Nessun task ha priorità inferiore → $B_{PIP} = 0$
- **$\tau_4$**: Può essere bloccato solo da $\tau_5$, su $R_1$ o $R_2$. Il tempo peggiore è 10 → $B_{PIP} = 10$
- **$\tau_3$**: Bloccabile da $\tau_5$ (su $R_1$: 10) e da $\tau_4$ (su $R_3$: 5) → $B_{PIP} = 10 + 5 = 15$
- **$\tau_2$**: Bloccabile da $\tau_5$ (su $R_1$: 10), $\tau_3$ (su $R_2$: 5) e $\tau_4$ (su $R_3$: 5) → $B_{PIP} = 10 + 5 + 5 = 20$
- **$\tau_1$**: Solo $\tau_3$ lo blocca su $R_2$ per 5 → $B_{PIP} = 5$

In PCP non si formano catene di bloccaggio. Ogni task subisce al più un bloccaggio, corrispondente al tempo massimo tra tutti quelli possibili.

---

|          | $R_1$ | $R_2$ | $R_3$ | $R_4$ | $R_5$ |  B<sub>PIP</sub> |  B<sub>PCP</sub> |
|----------|----|----|----|----|----|------------------|------------------|
| $\tau_1$ |  3 |  6 | 10 |    |  5 |               29 |               14 |
| $\tau_2$ |    |    |  8 |    |    |               23 |               14 |
| $\tau_3$ |    |  4 |    |  8 | 14 |               11 |               11 |
| $\tau_4$ |  7 |    |  9 |    | 11 |                  |                  |



In PIP, un task può essere bloccato da ogni risorsa utilizzata da task a priorità inferiore. Per ogni risorsa si considera al massimo un bloccaggio.

- **$\tau_1$**: Bloccabile da $\tau_4$ su $R_1$, $\tau_3$ su $R_2$, $\tau_4$ su $R_3$, $\tau_3$ e $\tau_4$ su $R_5$
$\rightarrow$ **B<sub>PIP</sub> =** $14 + 8 + 7 = 29$

- **$\tau_2$**: Bloccabile da $\tau_4$ su $R_1$ (7), $\tau_3$ su $R_2$, $\tau_4$ e $\tau_3$ su $R_3$ (9), $\tau_3$ su $R_5$ (14)  
$\rightarrow$ **B<sub>PIP</sub> =** $9 + 14 = 23$

- **$\tau_3$**: Bloccabile da $\tau_4$ su $R_1$ (7), $\tau_4$ su $R_3$ (9) e $\tau_4$ su $R_5$ (11)  
$\rightarrow$ **B<sub>PIP</sub> =** $\max{(7,9,11)}= 11$

- **$\tau_4$**: Nessun task ha priorità inferiore  
$\rightarrow$ **B<sub>PIP</sub> =** $0$

In PCP, un task subisce al massimo un bloccaggio, corrispondente al valore più alto tra quelli possibili.

--- 

|          | $R_1$ | $R_2$ | $R_3$ |  B<sub>PIP</sub>  |  B<sub>PCP</sub> |
|----------|----|----|----|-------------------|------------------|
| $\tau_1$ |  3 |  0 |  3 |                 9 |                6 |
| $\tau_2$ |  3 |  4 |  0 |                 6 |                6 |
| $\tau_3$ |  4 |  3 |  6 |                 0 |                0 |

In PIP, un task può essere bloccato da ogni risorsa utilizzata da task a priorità inferiore. Per ogni risorsa si considera al massimo un bloccaggio.

- **$\tau_1$**: Bloccabile da $\tau_2$ e $\tau_3$ su $R_1$, $R_3$. 
$\rightarrow$ **B<sub>PIP</sub> =** $6 + 3 = 9$
- **$\tau_2$**: Bloccabile da $\tau_3$ su $R_1$ e $R_2$
$\rightarrow$ **B<sub>PIP</sub> =** $6$ 
- **$\tau_3$**: Nessun task ha priorità inferiore
$\rightarrow$ **B<sub>PIP</sub> =** $0$

In PCP si considera solo il bloccaggio più alto tra le risorse a cui il task è esposto.

---