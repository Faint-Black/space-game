# Funcionalidades

## (1) Movimentação da Nave
A nave possui um sistema de "inércia", demorando um pouco para acelerar e parar, tanto para andar quanto para girar.

**Movimentação:**
- **W / S:** Acelera a nave para frente ou para trás.
- **A / D:** Move a nave para os lados.
- **Q / E:** Faz a nave subir ou descer verticalmente.

**Rotação:**
- **Setas Direcionais (Cima / Baixo):** Inclina o nariz da nave.
- **Setas Direcionais (Esquerda / Direita):** Vira a nave para os lados.
- **J / L:** Faz a nave girar no próprio eixo.

**Camera:**
- **C:** Alterna entre câmera 1ª pessoa e 3ª pessoa.
  - **Modo 3ª Pessoa (Orbital):** A câmera segue a nave de fora.
  - **Modo 1ª Pessoa (Cockpit):** A câmera é posicionada dentro da nave, olhando pelo vidro da frente.
- **K**: Fixa a camera no centro do objeto, acompanhando a rotação, mas "travando" o mouse.
- **Mouse:** Controla a rotação da câmera.


## (2) Sistemas Auxiliares

- **Canhões a Laser (Barra de Espaço):** Dispara projéteis em linha reta.
- **Braço Mecânico (G):** Um braço robótico com garra que pode ser estendido ou guardado.
- **Scanner (N):** Um holograma verde em formato de cone, projetado a partir da nave.


## (3) Modelos 3D
- **Campo Estelar:** O fundo do cenário possui diversas estrelas renderizadas aleatoriamente.
- **Modelo 3D:** A nave possui um modelo 3D texturizado.


## (4) HUD
- **Painel de Bordo (HUD):** Um marcador visual 2D sobreposto à tela, exibindo seus Pontos à esquerda e suas Vidas restantes à direita.
- **Colisões:** Estrutura inicial da HUD feita, falta implementar os asteroides e a detecção de colisão. 

---

# O que falta

## Nave e Controle (Grupo 1)

- **Melhorar a garra**
- **Melhorar a inércia da nave**

## Asteroides e Ambiente (Juntar com o código do Grupo 2)

- **Geração procedural de asteroides**
- **Armazenamento de vértices**
- **Implementação de AABB para cada asteróide**
- **Renderização e movimentação dos asteróides**

## HUD e Estados do jogo (Grupo 3)

- **Detecção de colisão com lasers**
- **Detecção de colisão com a nave**
- **Pause e Game Over**


