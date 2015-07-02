def parse_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Visualize Plane XZ of Graph")
    parser.add_argument('graph', help='Input Graph data file')
    parser.add_argument('output', help='Output Image file')
    parser.add_argument('--plane-y', type=int)
    return parser.parse_args()

def mainfunc():
    global ARGS
    ARGS = parse_args()

    import cv2, numpy as np, matplotlib.pyplot as plt
    

if __name__ == "__main__":
    mainfunc()
