using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace PatientMonitorSimulator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class PatientMonitor : Window
    {
        private readonly PatientMonitorViewModel _model;
        private DispatcherTimer _timer;
        public PatientMonitor(PatientMonitorViewModel model)
        {
            InitializeComponent();
            InitializeTimer();
            _model = model;
            DataContext = _model;
        }

        private void InitializeTimer()
        {
            _timer = new DispatcherTimer();
            _timer.Tick += Timer_Tick;
            _timer.Interval = new TimeSpan(0, 0, 5);
            _timer.Start();
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            _model.GenerateData();
        }
    }
}
