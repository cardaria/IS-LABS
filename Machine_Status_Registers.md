# 3.1.6 Machine Status Registers (mstatus och mstatush)

[cite_start]Registret `mstatus` är ett MXLEN-bitar stort läs- och skrivbart register som håller reda på och kontrollerar processorns (hartens) nuvarande operativa tillstånd[cite: 14, 15]. [cite_start]I S-mode finns en begränsad vy av detta register som kallas `sstatus`[cite: 16].

[cite_start]För **RV32**-system används även `mstatush`, ett 32-bitars register som innehåller de övre bitarna som motsvarar bit 62:36 i `mstatus` för RV64 (med undantag för fälten SD, SXL och UXL)[cite: 21, 22, 23].

---

## 3.1.6.1 Privilege and Global Interrupt-Enable Stack

[cite_start]Detta avsnitt hanterar hur globala avbrott och privilegienivåer sparas och återställs vid fällor (traps)[cite: 33].

### Globala Avbrott (Interrupt Enables)
* [cite_start]**MIE & SIE:** Globala bitar för att aktivera avbrott i M-mode respektive S-mode[cite: 34].
* **Funktion:**
    * Om $xIE = 1$ är avbrott globalt aktiverade för nivå $x$.
    * [cite_start]Om $xIE = 0$ är de inaktiverade[cite: 37].
    * [cite_start]Avbrott för lägre privilegienivåer ($w < x$) är alltid inaktiverade[cite: 37].
    * [cite_start]Avbrott för högre privilegienivåer ($y > x$) är alltid aktiverade[cite: 38].

### Privilegiestacken (The Stack)
För att stödja nästlade fällor har varje nivå en stack med två nivåer:
1.  [cite_start]**xPIE (Previous Interrupt Enable):** Sparar värdet av avbrottsbiten (IE) som var aktiv innan fällan togs[cite: 42].
2.  [cite_start]**xPP (Previous Privilege):** Sparar vilken privilegienivå processorn befann sig i innan fällan[cite: 42].
    * [cite_start]*Notera:* MPP är 2 bitar bred, SPP är 1 bit bred[cite: 43].

### Beteende vid Fälla (Trap Entry)
När en fälla tas från nivå $y$ till $z$:
1.  [cite_start]$zPIE$ sätts till värdet av $IE$[cite: 44].
2.  [cite_start]$zIE$ sätts till 0 (avbrott stängs av)[cite: 45].
3.  [cite_start]$PP$ sätts till $y$ (föregående nivå)[cite: 45].

### Beteende vid Återgång (Trap Return via xRET)
När instruktionen `MRET` eller `SRET` körs (förutsatt att PP håller $y$):
1.  [cite_start]$IE$ sätts till $PIE$[cite: 49].
2.  [cite_start]Privilegienivån ändras till $y$[cite: 50].
3.  [cite_start]$PIE$ sätts till 1[cite: 50].
4.  [cite_start]$PP$ sätts till den lägsta stödda nivån (t.ex. U eller M)[cite: 51].
5.  [cite_start]Om $xPP \neq M$, sätts `MPRV` till 0[cite: 52].

---

## 3.1.6.2 Base ISA Control

Dessa fält styr vilken ISA-bredd (XLEN) som gäller för olika privilegienivåer.

* [cite_start]**RV64:** Fälten **SXL** och **UXL** styr XLEN för S-mode respektive U-mode (samma kodning som `misa`-registret)[cite: 61, 62].
* [cite_start]**RV32:** Fälten SXL och UXL existerar inte (värdet är låst till 32)[cite: 64].
* [cite_start]**Datahantering:** Om XLEN sätts till ett smalare värde än hårdvarans maxbredd, ignoreras övre bitar vid läsning och resultat teckenexpanderas (sign-extended) vid skrivning[cite: 71, 72].

---

## 3.1.6.3 Memory Privilege

Fälten nedan modifierar rättigheterna för minnesåtkomst och används ofta för att effektivisera emulering eller OS-funktioner.

### MPRV (Modify PRiVilege)
* [cite_start]Påverkar **load** och **store** (men inte instruktionshämtning)[cite: 78, 83].
* **MPRV = 0:** Normalt beteende.
* [cite_start]**MPRV = 1:** Data-addresser översätts och skyddas som om processorn befann sig i privilegienivån angiven av **MPP**[cite: 80].

### MXR (Make eXecutable Readable)
* [cite_start]Påverkar läsbarheten av sidor i virtuellt minne[cite: 86].
* [cite_start]**MXR = 0:** Endast sidor markerade Readable ($R=1$) kan läsas via load[cite: 87].
* [cite_start]**MXR = 1:** Sidor markerade Readable ($R=1$) ELLER Executable ($X=1$) kan läsas[cite: 88].

### SUM (Permit Supervisor User Memory access)
* [cite_start]Påverkar S-mode åtkomst till U-mode minne[cite: 96].
* [cite_start]**SUM = 0:** S-mode får **inte** komma åt sidor markerade som User ($U=1$)[cite: 97].
* [cite_start]**SUM = 1:** S-mode tillåts komma åt User-sidor[cite: 98].

---

## 3.1.6.4 Endianness Control

[cite_start]Styr byte-ordningen (endianness) för dataåtkomster (instruktionshämtning är alltid little-endian)[cite: 104, 105].

| Fält | Beskrivning |
| :--- | :--- |
| **MBE** | [cite_start]Endianness för M-mode[cite: 106]. |
| **SBE** | Endianness för S-mode. [cite_start]Styr även implicita åtkomster (t.ex. sidtabellsläsning)[cite: 107, 109]. |
| **UBE** | [cite_start]Endianness för U-mode[cite: 108]. |

* **0:** Little-endian.
* **1:** Big-endian.

---

## 3.1.6.5 Virtualization Support

Dessa bitar tillåter hypervisors att fånga (trap) specifika operationer för att effektivt virtualisera gäst-OS.

### TVM (Trap Virtual Memory)
* [cite_start]**TVM = 1:** Om S-mode försöker ändra `satp` eller köra `SFENCE.VMA` / `SINVAL.VMA`, genereras en "Illegal Instruction Exception"[cite: 133].
* [cite_start]Syfte: Tillåter "lazy" uppdatering av skugg-sidtabeller (shadow page tables)[cite: 138].

### TW (Timeout Wait)
* [cite_start]**TW = 1:** Om `WFI` (Wait For Interrupt) körs i lägre privilegienivå och inte slutförs inom en viss tid, genereras ett undantag[cite: 142].
* [cite_start]Syfte: Förhindrar att en gäst "sover" inaktivt; hypervisorn kan byta till en annan gäst istället[cite: 144].

### TSR (Trap SRET)
* [cite_start]**TSR = 1:** Försök att köra `SRET` i S-mode genererar en "Illegal Instruction Exception"[cite: 149].
* [cite_start]Syfte: Nödvändigt för att emulera hypervisor-tillägg om hårdvarustöd saknas[cite: 151].

---

## 3.1.6.6 Extension Context Status

[cite_start]Fälten **FS**, **VS** och **XS** används för att optimera kontextbyten (context switching) genom att hålla reda på om utökade register (t.ex. flyttal) har ändrats ("Dirty") eller inte[cite: 155].

* [cite_start]**FS:** Floating-point status[cite: 156].
* [cite_start]**VS:** Vector status[cite: 157].
* [cite_start]**XS:** Övriga användartillägg[cite: 158].
* **SD:** En sammanfattande "Dirty"-bit (Read-only). [cite_start]Är 1 om något av FS, VS eller XS indikerar Dirty[cite: 180].

### Statuskodning

| Värde | Status | Betydelse för FS/VS | Betydelse för XS |
| :---: | :--- | :--- | :--- |
| **0** | **Off** | Avstängd (åtkomst ger exception) | [cite_start]All off [cite: 166] |
| **1** | **Initial** | Initialt värde (konstant) | [cite_start]None dirty/clean, some on [cite: 166] |
| **2** | **Clean** | Matchar värdet i minnet (ingen sparning krävs) | [cite_start]None dirty, some clean [cite: 166] |
| **3** | **Dirty** | Ändrat värde (måste sparas vid byte) | [cite_start]Some dirty [cite: 166] |