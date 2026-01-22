# ISA I3xM — Emulator/Simulator pentru un subset RISC-V 🛠️  

![Language](https://img.shields.io/badge/C-11-informational)
![License](https://img.shields.io/badge/License-MIT-blue)
![Pipeline](https://gitlab.com/<group>/<repo>/badges/main/pipeline.svg)
![Open Issues](https://img.shields.io/gitlab/issues/<group>/<repo>)

> Proiect didactic dezvoltat de **Boțoc Indi**, **Codreanu Mihai-Constantin**, **Isac Marco-Deian** și **Nedelea Maria-Paula**.

---

<details>
<summary>📑 Cuprins</summary>

1. [Introducere](#introducere)  
2. [Arhitectură generală](#arhitectură-generală)  
3. [Memorie, registre & flag-uri](#memorie-registre--flag-uri)  
4. [Setul de instrucțiuni](#setul-de-instrucțiuni)  
5. [Module software](#module-software)  
6. [UI & Interacțiune](#ui--interacțiune)  
7. [Assembler integrat](#assembler-integrat)  
8. [Compilare & rulare](#compilare--rulare)  
9. [Exemple](#exemple)  
10. [Structura repository-ului](#structura-repository-ului)  
11. [Limitări & direcții viitoare](#limitări--direcții-viitoare)  
12. [Bibliografie](#bibliografie)  
13. [Licență](#licență)  

</details>

---

## Introducere

**ISA I3xM** este un emulator/simulator scris în C 11 care reproduce ciclul _fetch → decode → execute_ pentru un procesor simplificat, inspirat de arhitectura RISC-V. Scopul principal este de a ilustra, în laborator, impactul fiecărei instrucțiuni asupra memoriei, registrelor și flag-urilor procesorului, utilizând o interfață **ncurses** în terminal :contentReference[oaicite:0]{index=0}.

---

## Arhitectură generală

┌───────────── ISA / UI (ncurses) ─────────────┐
│ Gestionare ferestre & evenimente tastatură │
├────────────────┬─────────────────────────────┤
│ PARSE_CODE │ ENCODER (bin out) │
│ (analizor asm)└─────┬───────────────────────┘
└───────────── DECODER/EXECUTOR ─────────┬─────┘
│

## MEMORIE & REGISTRE


* **Decoder** – mapează instrucțiunile brute în structuri interne.  
* **Encoder** – transformă instrucțiunile text în cod binar.  
* **Executor** – evaluează şi rulează instrucţiunile; actualizează memoria, registrele şi flag-urile.  
* **Memorie** – abstractizează segmentele instrucțiuni/date/stivă şi oferă operaţii sigure.  
* **Interfaţă** – afișează permanent codul, memoria şi registrele şi preia input-ul utilizatorului. :contentReference[oaicite:1]{index=1}

---

## Memorie, registre & flag-uri

| Componentă | Detalii |
|------------|---------|
| **Memorie segmentată** | Instrucțiuni, date și stivă sunt păstrate în zone contigue separate pentru claritate și protecție :contentReference[oaicite:2]{index=2} |
| **Registre** | 32 registre generale (`x0-x31`) + specialele `PC` (Program Counter) și `SP` (Stack Pointer) |
| **Flag-uri** | `ZF` – Zero, `SF` – Sign, `CF` – Carry, `OF` – Overflow; actualizate de instrucțiunile aritmetice/logice :contentReference[oaicite:3]{index=3} |

---

## Setul de instrucțiuni

Subsetul implementează tipurile **R**, **I** și **B** + câteva pseudo-instrucțiuni de stivă și memorie.  
Diagrama de câmpuri urmează convenția RISC-V, cu adaptări minore :contentReference[oaicite:4]{index=4}.

<details>
<summary>🔍 Instrucțiuni tip R</summary>

| Instrucțiune | OPCODE | FUNCT3 | FUNCT7 | Descriere |
|--------------|--------|--------|--------|-----------|
| `add` | `0110011` | `000` | `0000000` | Adunare registre |
| `sub` | `0110011` | `000` | `0100000` | Scădere registre |
| `mul` | `0110011` | `000` | `0000001` | Înmulțire registre |
| `udv` | `0110011` | `101` | `0000001` | Împărțire (cât) |
| `mod` | `0110011` | `110` | `0000001` | Împărțire (rest) |
| `lsl` | `0110011` | `001` | `0000000` | Shift logic stânga |
| `lsr` | `0110011` | `101` | `0000000` | Shift logic dreapta |
| `cmp` | `0110011` | `010` | `0000000` | Comparare registre |
| `and`, `xor`, `or`, `mov` … | … | … | … | Operații logice / mutare |  

</details>

<details>
<summary>🔍 Instrucțiuni tip I (+ pseudo-I)</summary>

| Instrucțiune | OPCODE (`0010011`) | FUNCT3 | Descriere |
|--------------|-------------------|--------|-----------|
| `addi`, `subi`, `xori`, `ori`, `andi` | `000`…`111` | Aritmetică imediat |
| `lsl`, `lsr`, `asr` | `001` / `101` | Shift cu imediate |
| `cmp` | `010` | SLTI‐based compare |
| `movi` | `000` | Mutare imediată |
| **Pseudo:** `lda`, `ldr`, `str`, `sta` | `0000011`/`0100011` | Load/Store addr/reg |
| `psh`, `pop`, `ret`, `hlt` | `0100011` / `0000011` / `1110011` | Stivă & control |  

</details>

<details>
<summary>🔍 Instrucțiuni tip B (branch)</summary>

| Instrucțiune | Condiție / scop | Descriere |
|--------------|-----------------|-----------|
| `bra` | — | Salt absolut |
| `beq`, `bne` | `rs1 == rs2`, `rs1 != rs2` | Salt condiționat pe egalitate |
| `blt`, `bge`, `bgt`, `ble` | Comparări semnate |
| `brz`, `brp`, `bmi` | Flag-uri `ZF`, `SF` |
| `bvs`, `bcs` | Flag-uri `OF`, `CF` |
| `jms` | Salt subrutină (push `PC`, branch) |  

</details>

---

## Module software

| Modul | Responsabilități principale |
|-------|-----------------------------|
| **ISA** | Inițializează/închide interfața, desenează ferestrele, primește input, trimite cod către analizor :contentReference[oaicite:8]{index=8} |
| **PARSE_CODE** | Extrage etichete, tokenizează liniile, formează un tabel de instrucțiuni  |
| **ENCODER** | Codifică R/I/B în format binar RISC-V adaptat :contentReference[oaicite:10]{index=10} |
| **DECODER/EXECUTOR** | Decodifică, actualizează flag-urile, rularea efectivă și ciclul procesorului :contentReference[oaicite:11]{index=11} |
| **MEMORY** | Acces aliniat, protecție segmente, push/pop stivă |
| **UI layer** | Ferestre separate: cod, memorie, registre, log execuție |

---

## UI & Interacțiune

* **ncurses** redă patru panouri: cod sursă, registre, memorie, log.  
* Control la runtime: `Space` (step), `Enter` (run/pause), `q` (quit).  
* Mesaje de eroare/avertizare apar într-un overlay modal :contentReference[oaicite:12]{index=12}.

---

## Assembler integrat

Emulatorul poate rula direct fișiere `.asm`; pipeline-ul intern:
.asm ─► PARSE_CODE ─► ENCODER (bin) ─► DECODER/EXECUTOR


Etichetele sunt rezolvate într-o primă trecere; instrucțiunile sunt puse într-un **string table** și apoi traduse în binar, după care sunt încărcate în segmentul _instrucțiuni_ al memoriei :contentReference[oaicite:13]{index=13}.

---

## Compilare & rulare

```bash
# Clonare
git clone https://gitlab.com/<group>/<repo>.git
cd isa-i3xm

# Build (Release)
make                         # sau  make DEBUG=1  pentru simboluri

# Rulare binar
./i3xm build/hello.bin

# Rulare direct din .asm
./i3xm examples/factorial.asm
```

Dependențe: GCC ≥ 9 (sau Clang), GNU Make, libncursesw5-dev pe Linux sau brew install ncurses pe macOS .

## Bibliografie:

Pentru bibliografie, puteti consulta pdf-ul din proiect
