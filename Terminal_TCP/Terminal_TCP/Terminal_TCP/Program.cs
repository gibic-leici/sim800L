using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

class ServidorTcp
{
    static void Main()
    {
        int puerto = 1050;
        Console.WriteLine("IP address: " + IPAddress.Any);
        TcpListener servidor = new TcpListener(IPAddress.Any, puerto);
        servidor.Start();
        Console.WriteLine("Servidor TCP iniciado en el puerto {0}", puerto);
        while (true)
        {
            TcpClient cliente = servidor.AcceptTcpClient();
            Console.WriteLine("Cliente conectado desde {0}", cliente.Client.RemoteEndPoint);
            NetworkStream stream = cliente.GetStream();
            byte[] buffer = new byte[1024];
            int bytesLeidos = stream.Read(buffer, 0, buffer.Length);
            string mensaje = Encoding.ASCII.GetString(buffer, 0, bytesLeidos);
            Console.WriteLine("Mensaje recibido: {0}", mensaje);
            byte[] respuesta = Encoding.ASCII.GetBytes("Respuesta recibida");
            stream.Write(respuesta, 0, respuesta.Length);
            stream.Close();
            cliente.Close();
            Console.WriteLine("Cliente desconectado");
        }
        servidor.Stop();
    }
}