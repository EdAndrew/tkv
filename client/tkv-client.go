package main

import "os"
import "net"
import "io"
import "fmt"
import "bytes"
import "flag"
import "strings"

var serverIp *string = flag.String("h", "localhost", "Ip address of tkv server")
var serverPort *string = flag.String("p", "6433", "Port of tkv server")

func main() {
	flag.Parse()
	server := *serverIp + ":" +  *serverPort

	conn, err := net.Dial("tcp", server);
	if err != nil {
		println(err.Error())
		os.Exit(-1)
	}

	var input string 
	for {
		//fmt.Scanf("%q", &input)
		input = "get 1"
		if strings.Compare(input, "quit") == 0 {
			break
		}
		input += "\n"

		_, err = conn.Write([]byte(input))
		if err != nil {
			println(err.Error())
			os.Exit(-1)
		}

		result, err := readFully(conn)
		if err != nil {
			println(err.Error())
			os.Exit(-1)
		}
		fmt.Println(string(result))
	}
	
	os.Exit(0)
}

func readFully(conn net.Conn)([]byte, error) {
	var buf [512]byte
	var cln []byte
	cln = []byte("\n")
	result := bytes.NewBuffer(nil)
	result.Reset()

	for {
		n, err := conn.Read(buf[0:])	
		if err != nil {
			if err == io.EOF {
				break
			}
			return nil, err
		}
		result.Write(buf[0:n])
		if buf[n-1] == cln[0] {
			break
		}
	}

	return result.Bytes(), nil
}
