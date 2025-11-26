const canvas = document.getElementById('myChart');
canvas.height = 100;

const onRefresh = chart => {
    const now = Date.now();
    chart.data.datasets.forEach(dataset => {
        dataset.data.push({
            x: now,
            // y: getRandomFloat(0.0, 3.3),
            y: currentEKG
        });
    });
};

const myChart = new Chart(canvas, {
    type: 'line',
    data: {
        datasets: [
            {
                label: 'Voltage (V)',
                backgroundColor: 'rgb(54, 162, 235)',
                borderColor: 'rgb(54, 162, 235)',
                cubicInterpolationMode: 'monotone',
                data: [],
            }
        ]
    },
    options: {
        animation: false,
        scales: {
            x: {
                type: 'realtime',
                realtime: {
                    duration: 4000,
                    refresh: 5,
                    delay: 0,
                    onRefresh: onRefresh
                }
            },
            y: {
                title: {
                    display: true,
                    text: 'Value'
                },
                min: 0,
                max: 3.3
            }
        },
        interaction: {
            intersect: false
        }
    }
});

function getRandomFloat(min, max) {
  return Math.random() * (max - min) + min;
}
