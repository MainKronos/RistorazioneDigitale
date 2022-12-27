<style>
	.grid {
		width: 100%;
		display: flex;
	}

	.mermaid {
		min-width: 400px;
	}
</style>


# RistorazioneDigitale

Applicazione distribuita basata sul paradigma client-server che implementa un 
sistema di prenotazione tavoli e gestione delle comande di un ristorante.

Il progetto contiene 5 cartelle: 
- 4 contengono i file per la logica di ogni dispositivo, e sono rispettivamente `Client`, `Server`, `TableDevice`, `KitchenDevice`
- La cartella `utils` contiene la logica e le costanti utilizzate da tutti i file del progetto

Ogni traferimento di dati tra i dispositivi avviene utilizzando il _binary protocol_, ogni informazione è definita e strutturata, e il numero di byte è prefissato.

Ogni volta che c'è uno scambio di informazioni tra dispositivo e server, o viceversa, viene prima inviato un comando che definisce l'operazione da svolgere e poi i rispettivi dati da trasmettere.

## Client
Il **Client** serve a inviare le prenotazioni al **Server**.
I comandi che vengono inviati al server sono:
- [`find`](#comando-find) per cercare i tavoli disponibili
- [`book`](#comando-book) per prenotare un tavolo

### Comando find

Invia una richiesta di disponibilità dove la data è espressa in formato GG-MM-AA e l’ora come HH.
Il server risponde con un elenco di opzioni di tavoli disponibili.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant cl as Client
	participant sv as Server

	cl->>sv: comando find
	Note over sv,cl: Invio dati prenotazione
	cl->>sv: cognome
	cl->>sv: numero persone
	cl->>sv: data e ora prenotazione
	Note over sv,cl: Ricezione tavoli
	sv->>cl: numero tavoli trovati
	loop Per ogni tavolo
		sv->>cl: ID tavolo
		sv->>cl: sala
		sv->>cl: ubicazione
	end
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `prenotazione::cognome` | 255 |
| 3 | `prenotazione::n_persone` | 2 |
| 4 | `prenotazione::datetime` | 4 |
| 5 | `len` | 4 |
| 6 | `tavolo::id` | 4 |
| 7 | `tavolo::sala` | 4 |
| 8 | `tavolo::ubicazione` | 255 |

</div>

### Comando book

Invia una richiesta di prenotazione di un tavolo corrispondente all'opzione scelta.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant cl as Client
	participant sv as Server

	cl->>sv: comando book
	cl->>sv: opzione scelta
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `len` | 4 |

</div>

## TableDevice

Il **TableDevice** serve a ordinare i piatti tramite le comande.
I comandi che vengono inviati al server sono:
- [`getid`](#comando-getid) per ottenere l'ID del tavolo
- [`unlock`](#comando-unlock) per sbloccare il tavolo
- [`menu`](#comando-menu) per ottenere il menu
- [`comanda`](#comando-comanda) per inviare la comanda
- [`conto`](#comando-conto) per richiedere il conto

### Comando getid

Richiede al server l'ID del tavolo.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant td as TableDevice
	participant sv as Server

	td->>sv: comando getid
	sv->>td: ID tavolo
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `tavolo_id` | 4 |

</div>

### Comando unlock

Convalida il codice di sblocco del tavolo inserito dall'utente.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant td as TableDevice
	participant sv as Server

	td->>sv: comando unlock
	td->>sv: ID tavolo
	td->>sv: codice
	sv->>td: stato sblocco
	sv->>td: messaggio
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `tavolo_id` | 4 |
| 3 | `unlock_code` | 4 |
| 4 | `uint8_t` | 1 |
| 5 | `response` | 2048 |

</div>

### Comando menu

Richiede al server il menu.

<div class="grid">

```mermaid

%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant td as TableDevice
	participant sv as Server

	td->>sv: comando menu
	sv->>td: numero piatti
	loop Per ogni piatto
		sv->>td: codice piatto
		sv->>td: nome piatto
		sv->>td: prezzo
	end
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `len` | 4 |
| 3 | `piatto::code` | 3 |
| 4 | `piatto::nome` | 255 |
| 5 | `piatto::prezzo` | 4 |

</div>

### Comando comanda

Invia al server la comanda.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant td as TableDevice
	participant sv as Server
	participant kd as KitchenDevice

	td->>sv: comando comanda
	td->>sv: ID tavolo
	td->>sv: numero comanda
	td->>sv: numero piatti
	loop Per ogni piatto
		td->>sv: codice piatto
		td->>sv: quantità
	end
	sv->>kd: comando numcom
	sv->>kd: variazione comande
	sv->>td: messaggio
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `tavolo_id` | 4 |
| 3 | `num_com` | 2 |
| 4 | `len` | 4 |
| 5 | `piatto::code` | 3 |
| 6 | `len` | 4 |
| 7 | `cmd` | 20 |
| 8 | `len` | 4 |
| 9 | `response` | 2048 |

</div>

### Comando conto

Richiede al server il conto.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant sv as Server
	participant td as TableDevice

	td->>sv: comando conto
	td->>sv: ID tavolo
	sv->>td: numero piatti
	loop Per ogni piatto
		sv->>td: codice piatto
		sv->>td: quantità
		sv->>td: totale parziale
	end
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `tavolo_id` | 4 |
| 3 | `len` | 4 |
| 4 | `type` | 3 |
| 5 | `len` | 4 |
| 6 | `valore` | 4 |

</div>

## KitchenDevice

Il **KitchenDevice** serve a gestire le comande. Le comande vengono prese in carico, preparate e servite.
I comandi che vengono inviati al server sono:
- [`getcomlen`](#comando-getcomlen) per ottenere il numero di comande in attesa
- [`take`](#comando-take) per prendere in carico una comanda in attesa da più tempo
- [`ready`](#comando-ready) per segnalare che la comanda è pronta

### Comando getcomlen

Richiede al server il numero di comande in attesa.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant kd as KitchenDevice
	participant sv as Server

	kd->>sv: comando getcomlen
	sv->>kd: numero comande
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `len` | 4 |

</div>

### Comando take

Prende in carico una comanda in attesa da più tempo.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant kd as KitchenDevice
	participant sv as Server
	participant td as TableDevice

	kd->>sv: comando take
	kd->>sv: ID tavolo
	kd->>sv: numero comanda
	kd->>sv: numero piatti
	loop Per ogni piatto
		kd->>sv: codice piatto
		kd->>sv: quantità
	end
	sv->>kd: comando numcom
	sv->>kd: variazione comande
	sv->>td: comando uptcom
	sv->>td: messaggio
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `tavolo_id` | 4 |
| 3 | `num_com` | 2 |
| 4 | `len` | 4 |
| 5 | `type` | 3 |
| 6 | `len` | 4 |
| 7 | `cmd` | 20 |
| 8 | `len` | 4 |
| 9 | `cmd` | 20 |
| 10 | `response` | 2048 |

</div>

### Comando ready

Segnala al server che la comanda è pronta.

<div class="grid">

```mermaid
%%{init: { "sequence": { "mirrorActors":false }}}%%

sequenceDiagram
	autonumber
	participant kd as KitchenDevice
	participant sv as Server
	participant td as TableDevice

	kd->>sv: comando ready
	kd->>sv: ID tavolo
	kd->>sv: numero comanda
	sv->>td: comando uptcom
	sv->>td: messaggio
```

| Op. | Tipo | N Byte |
|:---:|:-----|:-------|
| 1 | `cmd` | 20 |
| 2 | `tavolo_id` | 4 |
| 3 | `num_com` | 2 |
| 4 | `cmd` | 20 |
| 5 | `response` | 2048 |

</div>