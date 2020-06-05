using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;


namespace WindowsFormsApplication1
{
    public partial class FormPartida : Form
    {
        int dim;
        Socket server;
        int partida;
        int miFicha;
        int contador;
        delegate void delegado(string mensage);        

        public FormPartida(Socket server, int partida, int miFicha, int dim){
            InitializeComponent();
            this.server = server;
            this.partida = partida;
            this.miFicha = miFicha;
            this.dim = dim;
        }

        private void dGV_CellClick(object sender, DataGridViewCellEventArgs e){
            if ((contador!=-1)&&(miFicha == contador%dim)){
                if (dGV.CurrentCell.Value != null)
                    MessageBox.Show("CASILLA OCUPADA");
                else{
                    string mensaje = "8/" + Convert.ToString(partida)+ "/" + Convert.ToString(dGV.CurrentCell.RowIndex) + "/" + Convert.ToString(dGV.CurrentCell.ColumnIndex) + "/";
                    // Enviamos al servidor el nombre tecleado
                    byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                    server.Send(msg);
                }
            }else
                MessageBox.Show("Todavía no es tu turno!");
        }

        private void button9_Click(object sender, EventArgs e){//Chat
            string mensaje = "11/" + Convert.ToString(partida) + "/" + textBox2.Text;
            // Enviamos al servidor el nombre tecleado
            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);
        }

        private void FormPartida_Load(object sender, EventArgs e){
            this.label1.Text = "tu ficha es " + miFicha.ToString();
            this.dGV.RowCount = dim + 1;
            this.dGV.ColumnCount = dim + 1;
            contador = 0;
        }

        private void actualizarTablero(string datos){
            string[] dato = datos.Split(',');
            int fila = Convert.ToInt16(datos[0]);
            int colu = Convert.ToInt16(datos[1]);
            int juga = Convert.ToInt16(datos[2]);
            dGV[fila, colu].Value = juga;
        }

        private void avisoGanador(string ganador)
        {
            label1.Text = "ganador de la partida " + ganador;
            contador = -1;//per a que ningu pugui fer mes moviments
        }

        private void actualizarChat(string m){
            listBox1.BeginUpdate();
            listBox1.Items.Add(m);
            listBox1.EndUpdate();

        }

        public void recibirJugada(string datos)
        {
            dGV.Invoke(new delegado(actualizarTablero), new object[] { datos });
        }

        public void recibirGanador(string datos) {
            label1.Invoke(new delegado(avisoGanador), new object[] { datos });
        }

        public void recivirChat(string mssg)
        {
            listBox1.Invoke(new delegado(actualizarChat), new object[] { mssg });
        }
    }
}
