const canvas = document.getElementById('myChart');
canvas.height = 75;

// Initialize the chart 

const myChart = new Chart(canvas, {
    type: 'line',
    data: {
        labels: [1],
        datasets: [
            {
                label: 'Voltage (V)',
                yAxisID: 'A',
                backgroundColor: 'rgb(54, 162, 235)',
                borderColor: 'rgb(54, 162, 235)',
                data: [0],
            }
        ]
    },
    options: {
        animation: false,
        scales: {
            A: {
                type: 'linear',
                position: 'left',
                min: 0,
                max: 3.3
            },
        }
    }
});



function addData(chart, label, dataArray) {
    chart.data.labels.push(label);
    chart.data.datasets.forEach((dataset, index) => {
        dataset.data.push(dataArray[index]); // Add data for each dataset
    });
    chart.update();
}

let nextLabel = myChart.data.labels[myChart.data.labels.length-1]+1;

/*
// Add new data with incrementing labels every second
setInterval(function () {
    // pressure:2992,temp:-5

    // student pulls from serial getLine
    // parse it
    // TODO: student pulls from their own buffer for each variable
    
    const newData = [
        // Math.floor(Math.random() * 10), // Random data for Temperature
        // Math.floor(Math.random() * 20), // Random data for Pressure
        document.getElementById("ekg-value").innerText,
    ];
    addData(myChart, nextLabel++, newData);
    shiftChart(myChart);
}, 100);
*/

// shift data to the left, removing first point
function shiftChart(chart) {
    const maxDataPoints = 400; 
    if (chart.data.labels.length > maxDataPoints) {
        chart.data.labels.shift(); 
        chart.data.datasets.forEach((dataset) => {
            dataset.data.shift();
        });
    }
}
