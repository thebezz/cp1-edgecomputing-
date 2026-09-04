# 🌐 Wokwi Edge Computing

![VS Code](https://img.shields.io/badge/VS_Code-0078D4?style=for-the-badge&logo=visual%20studio%20code&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-F5822A?style=for-the-badge&logo=PlatformIO&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Wokwi](https://img.shields.io/badge/Wokwi_Simulator-181717?style=for-the-badge&logo=github&logoColor=white)

> Projeto focado no desenvolvimento e simulação de arquiteturas de **Edge Computing** voltadas para Internet das Coisas (IoT). O ambiente foi todo configurado para rodar localmente, combinando o poder do PlatformIO para gerenciamento de dependências/compilação e o Wokwi para simulação de hardware direto no editor.

---

## 🛠️ Pré-requisitos e Ferramentas

Para que o projeto funcione perfeitamente no seu ambiente local, você precisará instalar o **Visual Studio Code** e **duas extensões fundamentais**:

1. [**Visual Studio Code**](https://code.visualstudio.com/) - Editor principal.
2. 🐜 [**PlatformIO IDE**](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) - Extensão para compilar, fazer o upload e gerenciar as bibliotecas do microcontrolador.
3. ⚡ [**Wokwi Simulator**](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode) - Extensão oficial do Wokwi para simular o circuito e o código localmente sem precisar do hardware físico.

> **Aviso:** A extensão do Wokwi no VS Code exige ativação de uma licença gratuita. Siga os passos na própria extensão (ao rodar a primeira vez) para autenticar com seu navegador.

---

## 🚀 Como Configurar e Rodar o Projeto

Siga o passo a passo abaixo para rodar o simulador em sua máquina:

### 1. Clonar o Repositório
Abra o terminal e execute:
```bash
git clone https://github.com/snwvlr/Wokwi-Edge-Computing.git
cd Wokwi-Edge-Computing
```

### 2. Abrir o Projeto e Compilar
1. Abra a pasta do projeto no VS Code: `code .`
2. Aguarde o PlatformIO inicializar e instalar as dependências do `platformio.ini`.
3. Clique no ícone de **Check (✓)** na barra inferior do PlatformIO (Build) para compilar o código e garantir que não há erros de sintaxe.

### 3. Iniciar a Simulação no Wokwi
1. Com o código compilado com sucesso, abra o arquivo principal do seu circuito, geralmente chamado `diagram.json`.
2. Pressione `F1` (ou `Ctrl+Shift+P`) para abrir a paleta de comandos do VS Code.
3. Digite e selecione: **`Wokwi: Start Simulator`**.
4. A tela do simulador se dividirá ao lado do seu código. Sempre que você alterar o código e recompilar, o Wokwi irá atualizar automaticamente!

---

## 📁 Estrutura do Projeto

```text
Wokwi-Edge-Computing/
├── src/                # Código-fonte principal (.cpp, .c, .h)
├── lib/                # Bibliotecas locais do projeto
├── include/            # Arquivos de cabeçalho (.h)
├── diagram.json        # Arquivo de configuração visual do circuito Wokwi
├── wokwi.toml          # Arquivo de configuração de compilação do Wokwi
└── platformio.ini      # Arquivo de configuração do PlatformIO (placa, framework, libs)
```

---

## 👨‍💻 Autor

Desenvolvido por **[snwvlr](https://github.com/snwvlr)**.

Sinta-se à vontade para abrir uma *Issue* ou enviar um *Pull Request* caso tenha sugestões de melhoria para a arquitetura de Edge Computing!
