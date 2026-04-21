
#files to use for new csv and graphs
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import math
#CHANGE TO PROPER CSV FILE NAME AFTER DONE
railway = None
railway = pd.read_csv('ON site w damp.CSV')


# Make sure Distance is numeric
railway["Distance"] = pd.to_numeric(railway["Distance"], errors="coerce")

# Remove rows where Distance is blank/bad
railway = railway.dropna(subset=["Distance"]).copy()


#fix distance offset and roll everything back by first points distance from zero

dStart = railway['Distance'].iloc[0]

#rewrite to roll back everything from distance
railway['Distance'] = railway['Distance'] - dStart


#sort railways into sectioned
#240 inches per section and will devide as an int to sort into section number
railway["Section Number"] = (railway["Distance"] // 240).astype(int) + 1
railway["Section"] = "Section " + railway["Section Number"].astype(str)

#test to ensure that shows first 5 and last 5 sections and ammount of rows in each section
#print(railway["Section Number"].value_counts().sort_index())

#conversion for accelerometer
railway["axNew"] = railway['AX'] / 16384.0
railway['ayNew'] = railway['AY'] / 16384.0
railway['azNew'] = railway['AZ'] / 16384.0

#gyroscope conversion into degrres/s

railway['gxNew'] = railway['GX'] / 131.0
railway['gyNew'] = railway['GY'] / 131.0
railway['gzNew'] = railway['GZ'] / 131.0

#complimentary filtering for tilt

#accelerometer angle
railway["accelangle"] = np.atan2(railway['ayNew'], railway['azNew'])
#convert values from raidans to angles
railway["accelangle"] = np.degrees(railway["accelangle"])


#calculate the offset using first 50 data points
accelOffSet = railway["accelangle"].iloc[50].mean()

#use the offset to get accurate measurement
railway['accelangle'] = railway['accelangle'] - accelOffSet

#gyroscope
gyroOffSet = railway['gxNew'].iloc[:300].mean()
railway['gyroangle'] = railway['gxNew'] - gyroOffSet
#time derivative for filtering
railway['dt'] = railway['Time'].diff() / 1000
railway['dt'] = railway['dt'].fillna(0)

# integrate gyro angle
theta_gyro = [railway['accelangle'].iloc[0]]

for i in range(1, len(railway)):
    previous = theta_gyro[-1]
    omega = railway['gyroangle'].iloc[i]
    dt = railway['dt'].iloc[i]

    theta_gyro.append(previous + omega * dt)

railway['theta_gyro'] = theta_gyro

# complementary filtering
alpha = 0.99
theta = [railway['accelangle'].iloc[0]]

for i in range(1, len(railway)):
    previous = theta[-1]
    omega = railway['gyroangle'].iloc[i]
    dt = railway['dt'].iloc[i]
    accel = railway['accelangle'].iloc[i]

    theta_new = alpha * (previous + omega * dt) + (1 - alpha) * accel
    theta.append(theta_new)

railway['theta_comp'] = theta

# save processed to csv
railway.to_csv("processed_railway.csv", index=False)


section_max = railway.groupby("Section Number")["theta_comp"].apply(
    lambda x: np.max(np.abs(x))
)

worst_sections = list(section_max.nlargest(5).index)

plt.figure(figsize=(16, 6))
plt.plot(railway["Distance"], railway["theta_comp"], label="Complementary Filter", linewidth=2)

num_sections = int(railway["Section Number"].max())
section_length = 240

for s in range(num_sections):
    x_start = s * section_length
    x_end = (s + 1) * section_length
    section_num = s + 1

    if section_num in worst_sections:
        plt.axvspan(
            x_start,
            x_end,
            alpha=0.20,
            label="Worst Section" if section_num == worst_sections[0] else ""
        )
    else:
        if s % 2 == 0:
            plt.axvspan(x_start, x_end, alpha=0.05)

    plt.axvline(x=x_start, linestyle='--', alpha=0.25)

plt.xlabel("Distance")
plt.ylabel("Angle (degrees)")
plt.title("Complementary Filter Roll Angle with Worst Sections")
plt.legend()
plt.grid(True)
plt.show()