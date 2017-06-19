package main

import "os"
import "net"
import "io"
import "fmt"
import "bytes"
import "flag"

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
	fmt.Scanf("%q", &input)
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
	
	os.Exit(0)
}

func readFully(conn net.Conn)([]byte, error) {
	defer conn.Close()
	var buf [512]byte
	var cln []byte
	cln = []byte("\n")
	result := bytes.NewBuffer(nil)

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
