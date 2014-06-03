#include <iostream>
#include <string>
#include <thread>
#include <string.h>
using namespace std;
#include <netlink/socket.h>
#include <netlink/socket_group.h>
const string HOST = "localhost";
const unsigned SERVER_PORT = 5000;
bool disconnect = false;
vector<string> userInput;

class OnRead: public NL::SocketGroupCmd {
        void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference)
        {
            char buffer[256];
            buffer[255] = '\0';
            socket->read(buffer, 255);
            
            if (buffer[0] == 0)
            {
                Rysuj_plansze(buffer);
            }
            else
            {
                cout << "\nReceived message: " << buffer << endl;
                cout.flush();
            }
        }
        void Rysuj_plansze(char t[])
        {
            for (int i = 1; i <= 9; i++)
            {
                cout << " " << t[i] << " ";
                if (i % 3)
                    cout << "|";
                else if (i != 9)
                    cout << "\n---+---+---\n";
                else cout << endl;
            }
    }
};

class OnDisconnect: public NL::SocketGroupCmd {
        void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {
                disconnect = true;
        }
};

void inputGetter() {
	char buffer[1000] = {0};
	while (!disconnect) {
		cout.flush();
		cin.getline(buffer, 1000);
		userInput.push_back(buffer);
		if (userInput.back() == "exit")
			break;
	}

}

int main() {
        cout << "\nConnecting...";
        cout.flush();
        thread igetter(inputGetter);
        try {
                NL::init();
                NL::Socket socket(HOST, SERVER_PORT);
                NL::SocketGroup group;
                group.add(&socket);
                OnRead onRead;
                OnDisconnect onDisconnect;
                group.setCmdOnRead(&onRead);
                group.setCmdOnDisconnect(&onDisconnect);
                while(!disconnect) {
                	if (userInput.size() > 0){
                		string input = userInput.back();
                		userInput.pop_back();
						if(input == "exit")
                        	disconnect = true;
                    	else
	                        socket.send(input.c_str(), input.size()+1);                		
                	}
                	
                    group.listen(500);
                }
        }
        catch(NL::Exception e) {
                cout << "\n***ERROR*** " << e.what();
                disconnect = true;
        }
        igetter.join();
}
