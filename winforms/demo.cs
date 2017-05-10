using System.Drawing;
using System.Windows.Forms;

namespace W32CtlTest
{
    public class Demo : Form
    {
        private FlowLayoutPanel panel;
        private Button button;
        private TextBox textBox;

        public Demo() : base()
        {
            Text = "winforms";
            panel = new FlowLayoutPanel();

            button = new Button();
            button.Text = "test";
            button.Click += (sender, args) =>
            {
                Close();
            };
            panel.Controls.Add(button);

            textBox = new TextBox();
            panel.Controls.Add(textBox);

            Controls.Add(panel);
        }

        protected override Size DefaultSize
        {
            get
            {
                return new Size(240,100);
            }
        }

        public static void Main(string[] argv)
        {
            if (argv.Length < 1 || argv[0] != "-s")
            {
                Application.EnableVisualStyles();
            }
            Application.Run(new Demo());
        }
    }
}
