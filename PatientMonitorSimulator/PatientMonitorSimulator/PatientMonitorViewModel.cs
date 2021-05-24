using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace PatientMonitorSimulator
{
    public class PatientMonitorViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string propertyName)
        {
            var changed = PropertyChanged;

            if (changed == null)
                return;

            changed.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private string _nibp = "122 / 78";

        public string Nibp
        {
            get { return _nibp; }
            set 
            {
                _nibp = value;
                OnPropertyChanged("Nibp");
            }
        }

        private string _sp02 = "95";

        public string SpO2
        {
            get { return _sp02; }
            set
            {
                _sp02 = value;
                OnPropertyChanged("SpO2");
            }
        }

        private string _pulse = "66";

        public string Pulse
        {
            get { return _pulse; }
            set
            {
                _pulse = value;
                OnPropertyChanged("Pulse");
            }
        }

        private string _rr = "17";

        public string RR
        {
            get { return _rr; }
            set
            {
                _rr = value;
                OnPropertyChanged("RR");
            }
        }

        private string _t = "36.7";

        public string T
        {
            get { return _t; }
            set
            {
                _t = value;
                OnPropertyChanged("T");
            }
        }

        public void GenerateData()
        {
            int nibp1 = new Random().Next(120, 135);
            int nibp2 = new Random().Next(70, 85);
            Nibp = $"{nibp1} / {nibp2}";

            int sp = new Random().Next(85, 105);
            SpO2 = sp.ToString();

            int pulse = new Random().Next(65, 89);
            Pulse = pulse.ToString();

            int rr = new Random().Next(10, 22);
            RR = rr.ToString();

            double temp = 35 + new Random().NextDouble() * 5;
            T = Math.Round(temp, 1).ToString();
        }
    }
}
