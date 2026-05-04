# ⚡ cli-chat-server

> Um servidor de chat TCP robusto, multithread e direto no terminal.
> Desenvolvido em **C** para ambientes Linux.

---

## 🚀 Funcionalidades Principais

* 🧵 **Arquitetura Multithread**
  Gestão de múltiplos usuários simultâneos via `pthread`.

* 🏠 **Sistema de Salas**
  Isolamento de mensagens por canais dinâmicos (`/join`).

* 💬 **Mensagens Privadas**
  Comunicação direta entre usuários com `/msg`.

* 📝 **Logs em Tempo Real**
  Registro automático de conversas em `server/logs/`.

* 🔒 **Segurança**
  Uso de mutex para evitar condições de corrida.

---

## 🏗️ Arquitetura do Sistema

O projeto utiliza **sockets TCP** com o modelo:

> 🧠 *One Thread per Client*

Cada cliente conectado recebe uma thread dedicada, garantindo comunicação simultânea sem bloqueios.

---

## 🎯 Setup Rápido

### 🔧 1. Ajuste de IP & Porta

Configure no `Makefile`:

```c
IP = 0.0.0.0     # Aceita conexões de qualquer interface
PORT = 8996      # Porta do servidor
```

---

### 🚀 2. Compilar e iniciar

```bash
make clean && make server
./server/server 0.0.0.0 8996
```

---

## 🤝 Como se conectar

Use o **Netcat (nc)** em outro terminal ou máquina:

```bash
nc 172.16.41.103 8996
```

> 💡 **Dica:** descubra seu IP com `hostname -I`

> 🚨 **Se não conectar:**
> libere a porta com:

```bash
sudo ufw allow 8996/tcp
```

---

## 💬 Comandos Disponíveis

| Comando             | Descrição                  | Exemplo           |
| ------------------- | -------------------------- | ----------------- |
| `/nick <nome>`      | Define seu apelido no chat | `/nick Fabricio`  |
| `/join <sala>`      | Entra em uma sala          | `/join ADS`       |
| `/leave`            | Sai da sala atual          | `/leave`          |
| `/msg <nick> <msg>` | Envia mensagem privada     | `/msg Amigo Ola!` |

---

## 🧪 Exemplo de uso

```bash
[SISTEMA] Você entrou na sala: ADS
João: Fala, galera de sistemas!
```

---

## 📂 Estrutura do Projeto

```bash
server/
├── src/        # Lógica principal e comandos
├── include/    # Arquivos .h
└── logs/       # Histórico das conversas
```

---

## 🛠️ Tecnologias

* Linguagem C
* Sockets TCP (POSIX)
* Threads (`pthread`)
* Mutex (controle de concorrência)

---

## 📄 Licença

MIT License
