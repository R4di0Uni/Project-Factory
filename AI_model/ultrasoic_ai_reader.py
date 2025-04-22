import numpy as np
import pandas as pd



def load_sensor_data(file_path):
    """Load ultrasonic sensor data from a CSV file."""
    data = pd.read_csv(file_path, header=None)  
    return data.values  
def predict_distance(sensor_data):
    """Use the AI model to predict the distance based on sensor data."""
    sensor_data = np.reshape(sensor_data, (1, -1))  
    prediction = model.predict(sensor_data)
    return prediction[0][0]  
def main():
   
    file_path = 'sensor_data.csv' 
    sensor_data = load_sensor_data(file_path)

    
    for i, data_point in enumerate(sensor_data):
        print(f"Processing data point {i + 1}: {data_point}")
        
       
        predicted_distance = predict_distance(data_point)
        
       
        print(f"Predicted distance: {predicted_distance} cm")

if __name__ == "__main__":
    main()
