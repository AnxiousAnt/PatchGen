import pygame
import time
import random

# Initialize Pygame
pygame.init()

# Set up some constants
WIDTH, HEIGHT = 800, 600
SPEED = 10
BLOCK_SIZE = 20

# Set up some variables
snake = [(200, 200), (220, 200), (240, 200)]
direction = 'RIGHT'
food = [(400, 300), (500, 400), (600, 500)]
score = 0

# Set up the display
screen = pygame.display.set_mode((WIDTH, HEIGHT))

# Set up the clock
clock = pygame.time.Clock()

# Set up the font for the score
font = pygame.font.Font(None, 36)

# Game loop
while True:
    # Event handling
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            quit()
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_UP and direction != 'DOWN':
                direction = 'UP'
            elif event.key == pygame.K_DOWN and direction != 'UP':
                direction = 'DOWN'
            elif event.key == pygame.K_LEFT and direction != 'RIGHT':
                direction = 'LEFT'
            elif event.key == pygame.K_RIGHT and direction != 'LEFT':
                direction = 'RIGHT'

    # Move the snake
    head = snake[-1]
    if direction == 'UP':
        new_head = (head[0], head[1] - BLOCK_SIZE)
    elif direction == 'DOWN':
        new_head = (head[0], head[1] + BLOCK_SIZE)
    elif direction == 'LEFT':
        new_head = (head[0] - BLOCK_SIZE, head[1])
    elif direction == 'RIGHT':
        new_head = (head[0] + BLOCK_SIZE, head[1])
    snake.append(new_head)

    # Check for collision with food
    for i, f in enumerate(food):
        if snake[-1] == f:
            food.pop(i)
            score += 1
            break
    else:
        snake.pop(0)

    # Add new food
    if len(food) < 3:
        food.append((random.randint(0, WIDTH - BLOCK_SIZE) // BLOCK_SIZE * BLOCK_SIZE,
                     random.randint(0, HEIGHT - BLOCK_SIZE) // BLOCK_SIZE * BLOCK_SIZE))

    # Check for collision with wall or self
    if (snake[-1][0] < 0 or snake[-1][0] >= WIDTH or
        snake[-1][1] < 0 or snake[-1][1] >= HEIGHT or
        snake[-1] in snake[:-1]):
        pygame.quit()
        quit()

    # Draw everything
    screen.fill((0, 0, 0))
    for i, pos in enumerate(snake):
        if i == len(snake) - 1:  # Head of the snake
            pygame.draw.rect(screen, (255, 255, 0), pygame.Rect(pos[0], pos[1], BLOCK_SIZE, BLOCK_SIZE))
            # Draw eyes
            pygame.draw.circle(screen, (0, 0, 0), (pos[0] + BLOCK_SIZE // 3, pos[1] + BLOCK_SIZE // 3), BLOCK_SIZE // 6)
            pygame.draw.circle(screen, (0, 0, 0), (pos[0] + BLOCK_SIZE * 2 // 3, pos[1] + BLOCK_SIZE // 3), BLOCK_SIZE // 6)
        else:
            pygame.draw.rect(screen, (255, 255, 0), pygame.Rect(pos[0], pos[1], BLOCK_SIZE, BLOCK_SIZE))
    for f in food:
        pygame.draw.rect(screen, (255, 0, 0), pygame.Rect(f[0], f[1], BLOCK_SIZE, BLOCK_SIZE))
    score_text = font.render(f'Score: {score}', True, (255, 255, 255))
    screen.blit(score_text, (WIDTH - 150, HEIGHT - 50))
    pygame.display.flip()

    # Cap the frame rate
    clock.tick(SPEED)