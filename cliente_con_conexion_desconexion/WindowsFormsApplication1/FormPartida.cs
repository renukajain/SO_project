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
        int partida;
        Socket server;
        int miFicha;
        public FormPartida(int partida, Socket server, int miFicha)
        {
            InitializeComponent();
            this.partida = partida;
            this.server = server;
            this.miFicha = miFicha;
        }

        private void FormPartida_Load(object sender, EventArgs e)
        {
            label1.Text = "Partida: " + this.partida.ToString() + " tu ficha " + this.miFicha.ToString();
            this.dataGridView1.RowCount = 3;
            this.dataGridView1.ColumnCount = 3;
        }

        private void dataGridView1_CellContentClick(object sender, DataGridViewCellEventArgs e)//en viamos posiciones
        {
            // codigoServicio / IDpartida/ jugador/ fila/columna
            string mensaje = "12/" + Convert.ToString(this.partida) + "/" + this.miFicha + e.RowIndex.ToString() + "/" + e.ColumnIndex.ToString();
            // Enviamos al servidor el nombre tecleado
            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);
        }
    }
}
