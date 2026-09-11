#include <fstream>   // fornece classes para trabalhar com arquivos,
                     // como std::ifstream (leitura) e std::ofstream (escrita)

#include <iostream>  // fornece entrada e saída padrão,
                     // como std::cout e std::cin

#include <string>    // fornece a classe std::string

#include <sstream>   // fornece std::stringstream,
                     // que permite tratar uma string como um stream


#include <vector>      

//por norma structs são publicas e classes privadas, quando se trata dos seus atibutos; 
//só usa classe se precisar de encapsulamento(gettrs e setters)
struct Servidor{
    std::string ip;
    int porta=0;
};

int main() {

    // std::ifstream = "input file stream"
    // abre o arquivo hotuk.ini para leitura
    std::ifstream arquivo("hotuk.ini");

    if (!arquivo) {
        // std::cout envia texto para a saída padrão (terminal)
        std::cout << "Nao foi possivel abrir o arquivo\n";
        return 1;
    }


   
    std::string linha;



 
    std::vector<Servidor> servidores;


    // std::getline lê uma linha inteira do stream "arquivo"
    while (std::getline(arquivo, linha))
    {

        // cria um stream baseado no conteúdo da string "linha".       
        // parser passa a permitir leitura com >>
        std::stringstream parser(linha);

        std::string pal;
        // extrai o primeiro token do parser e coloca em "pal".
        parser >> pal;


        // compara a primeira palavra da linha com "*CLAN_SERVER"
        if (pal == "*CLAN_SERVER")
        {
            Servidor server;

            while (parser >>  server.ip >> server.porta)
            {
                servidores.push_back(server);
            }
        }


    }


    for (size_t i = 0; i < servidores.size(); i++)
    {
        std::cout << servidores[i].ip << "\n";
        std::cout << servidores[i].porta << "\n";
    }
    


    return 0;
}