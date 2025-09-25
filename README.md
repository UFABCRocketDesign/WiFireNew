# WiFIre

O **WiFIre** é um sistema embarcado em desenvolvimento pelo **Departamento de Sistemas Embarcados**, projetado para o controle remoto seguro de ignição a partir de comunicação via **Wi-Fi**.  

O sistema trabalha com envio de **pings e respostas**, além de comandos de **armamento, desarmamento e ignição**, utilizando um conjunto de botões coloridos que representam cada ação.

---

## 🎛️ Botões de Controle

- 🟡 **Amarelo** → Envia comando de **teste (ping)**, reseta a senha e desarma os sistemas.  
- 🟢 **Verde** → Utilizado na lógica de senha e sequência de armamento.  
- 🔵 **Azul** → Utilizado na lógica de senha e sequência de armamento.  
- 🔴 **Vermelho** → Aciona o comando de **ignição** (apenas em estado armado).  

---

## 📡 Comandos de Comunicação

O protocolo de comunicação é baseado em mensagens simples de texto:

- `PING!\n` → Comando de teste enviado pela base.  
- `PONG!\n` → Resposta do sistema indicando comunicação ativa.  
- `ARMED!\n` → Coloca o sistema em **estado armado** (após senha correta).  
- `DISARMED!\n` → Retorna o sistema para **estado desarmado**.  
- `IGN!\n` → Executa o comando de **ignição** (apenas se armado).  

---

## 🔐 Senha de Armamento

Para transitar do estado **desarmado** para o estado **armado**, é necessário inserir a seguinte sequência nos botões:

B G B B G


Ou seja: **Azul → Verde → Azul → Azul → Verde**.  
Somente após o reconhecimento correto da senha o sistema aceita o comando de ignição.  

---

## ⚠️ Status Atual

- O **código já está estável e funcional**.  
- O sistema conta com diversos **modos de operação**, configurados por meio das macros:  
  - **PROPER_BOARD** → usado nas placas com botões integrados, funcionando como interface completa de controle.  
  - **NEW_BOARD** → modo específico para a nova placa TE 2025, com alterações de pinos e adição da função de **teste de continuidade do ignitor**.  
  - **Sem PROPER_BOARD** → utilizado em placas de voo que fazem apenas o acionamento de ignição. Esse modo é pensado como **backup**, caso seja necessário acionar utilizando a placa de voo.  
- O sistema também possui **modos configuráveis para buzzer** e opções para **ativar/desativar o LED RGB**.  

---

