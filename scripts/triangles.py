from dataclasses import dataclass
from matplotlib import pyplot as plt
from math import sqrt

@dataclass
class v2:
    X: float
    Y: float

    def module(self):
        return sqrt(self.X * self.X + self.Y * self.Y)
    
    def __add__(self, other):
        return v2(self.X + other.X, self.Y + other.Y)
    
    def __sub__(self, other):
        return v2(self.X - other.X, self.Y - other.Y)

def cross(v: v2, w: v2):
    return v.X * w.Y - v.Y * w.X

@dataclass
class Triangle:
    p0: v2
    p1: v2
    p2: v2

    def plot(self):
        plt.plot([self.p0.X, self.p1.X, self.p2.X, self.p0.X], [self.p0.Y, self.p1.Y, self.p2.Y, self.p0.Y], marker='.')

    def area(self):
        return 0.5 * cross(self.p1 - self.p0, self.p2 - self.p0)

    def isInside(self, point: v2):
        area1 = Triangle(self.p0, self.p1, point).area()
        area2 = Triangle(self.p0, point, self.p2).area()
        area3 = Triangle(point, self.p1, self.p2)

        return 

    def intersect(self, other):
        

def intersect(t1: Triangle, t2: Triangle):
    pass

if __name__ == "__main__":
    t1 = Triangle(v2(0,0), v2(1,0), v2(1,1))

    t1.plot()
    plt.show()
