import torch
import torch.nn as nn
import torch.optim as optim

class TradingModel(nn.Module):
    def __init__(self, input_size, output_size):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(input_size, 64),
            nn.ReLU(),
            nn.Linear(64, output_size)
        )

    def forward(self, x):
        return self.net(x)

model = TradingModel(input_size=5, output_size=3)
optimizer = optim.Adam(model.parameters(), lr=1e-3)
loss_fn = nn.MSELoss()

def predict(inputs):
    with torch.no_grad():
        return model(inputs).numpy()

def train_step(inputs, targets):
    optimizer.zero_grad()
    outputs = model(inputs)
    loss = loss_fn(outputs, targets)
    loss.backward()
    optimizer.step()
    return loss.item()

def save_model(path="model.pt"):
    torch.save(model.state_dict(), path)

def load_model(path="model.pt"):
    model.load_state_dict(torch.load(path))
