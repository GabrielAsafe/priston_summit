// Navegação suave e efeitos de scroll
document.addEventListener('DOMContentLoaded', function() {
    // Navbar scroll effect
    const navbar = document.querySelector('.navbar');
    
    window.addEventListener('scroll', function() {
        if (window.scrollY > 100) {
            navbar.classList.add('scrolled');
        } else {
            navbar.classList.remove('scrolled');
        }
    });

    // Smooth scrolling para links de navegação
    const navLinks = document.querySelectorAll('.nav-link');
    
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const targetId = this.getAttribute('href');
            const targetSection = document.querySelector(targetId);
            
            if (targetSection) {
                targetSection.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        });
    });

    // Hamburger menu toggle
    const hamburger = document.querySelector('.hamburger');
    const navMenu = document.querySelector('.nav-menu');
    
    hamburger.addEventListener('click', function() {
        navMenu.classList.toggle('active');
        hamburger.classList.toggle('active');
    });

    // Particles animation
    createParticles();
    
    // Counter animation
    animateCounters();
    
    // AOS-like animation
    initScrollAnimations();
    
    // Class cards interaction
    initClassCards();
    
    // Parallax effect
    initParallax();
    
    // Floating elements
    initFloatingElements();
});

// Criar partículas animadas
function createParticles() {
    const particlesContainer = document.getElementById('particles');
    const particleCount = 50;
    
    for (let i = 0; i < particleCount; i++) {
        const particle = document.createElement('div');
        particle.className = 'particle';
        
        // Posição aleatória
        particle.style.left = Math.random() * 100 + '%';
        particle.style.top = Math.random() * 100 + '%';
        
        // Tamanho aleatório
        const size = Math.random() * 4 + 2;
        particle.style.width = size + 'px';
        particle.style.height = size + 'px';
        
        // Delay de animação aleatório
        particle.style.animationDelay = Math.random() * 6 + 's';
        particle.style.animationDuration = (Math.random() * 4 + 4) + 's';
        
        particlesContainer.appendChild(particle);
    }
}

// Animação de contadores
function animateCounters() {
    const counters = document.querySelectorAll('.stat-number');
    const speed = 200;
    
    const animateCounter = (counter) => {
        const target = parseInt(counter.getAttribute('data-target'));
        const count = parseInt(counter.innerText);
        const increment = target / speed;
        
        if (count < target) {
            counter.innerText = Math.ceil(count + increment);
            setTimeout(() => animateCounter(counter), 1);
        } else {
            counter.innerText = target;
        }
    };
    
    // Intersection Observer para iniciar animação quando visível
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                const counter = entry.target;
                if (!counter.classList.contains('animated')) {
                    counter.classList.add('animated');
                    animateCounter(counter);
                }
            }
        });
    });
    
    counters.forEach(counter => {
        observer.observe(counter);
    });
}

// Animações de scroll (AOS-like)
function initScrollAnimations() {
    const animatedElements = document.querySelectorAll('[data-aos]');
    
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('aos-animate');
            }
        });
    }, {
        threshold: 0.1,
        rootMargin: '0px 0px -50px 0px'
    });
    
    animatedElements.forEach(element => {
        observer.observe(element);
    });
}

// Interação com cards de classes
function initClassCards() {
    const classCards = document.querySelectorAll('.class-card');
    
    classCards.forEach(card => {
        card.addEventListener('click', function() {
            // Remove active de todos os cards
            classCards.forEach(c => c.classList.remove('active'));
            // Adiciona active ao card clicado
            this.classList.add('active');
        });
        
        // Efeito de hover com mouse tracking
        card.addEventListener('mousemove', function(e) {
            const rect = this.getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;
            
            const centerX = rect.width / 2;
            const centerY = rect.height / 2;
            
            const rotateX = (y - centerY) / 10;
            const rotateY = (centerX - x) / 10;
            
            this.style.transform = `perspective(1000px) rotateX(${rotateX}deg) rotateY(${rotateY}deg) translateY(-10px)`;
        });
        
        card.addEventListener('mouseleave', function() {
            this.style.transform = 'perspective(1000px) rotateX(0) rotateY(0) translateY(0)';
        });
    });
}

// Efeito parallax
function initParallax() {
    window.addEventListener('scroll', function() {
        const scrolled = window.pageYOffset;
        const parallaxElements = document.querySelectorAll('.hero-background, .world-background');
        
        parallaxElements.forEach(element => {
            const speed = 0.5;
            element.style.transform = `translateY(${scrolled * speed}px)`;
        });
    });
}

// Elementos flutuantes
function initFloatingElements() {
    const floatingElements = document.querySelectorAll('.floating');
    
    floatingElements.forEach((element, index) => {
        element.style.animationDelay = (index * 0.5) + 's';
    });
}

// Efeito de digitação para o título
function typeWriter(element, text, speed = 100) {
    let i = 0;
    element.innerHTML = '';
    
    function type() {
        if (i < text.length) {
            element.innerHTML += text.charAt(i);
            i++;
            setTimeout(type, speed);
        }
    }
    
    type();
}

// Efeito de glitch no título
function glitchEffect() {
    const title = document.querySelector('.game-title');
    const originalText = title.textContent;
    const glitchChars = '!@#$%^&*()_+-=[]{}|;:,.<>?';
    
    let iterations = 0;
    const maxIterations = 10;
    
    const interval = setInterval(() => {
        title.textContent = originalText
            .split('')
            .map((char, index) => {
                if (index < iterations) {
                    return originalText[index];
                }
                return glitchChars[Math.floor(Math.random() * glitchChars.length)];
            })
            .join('');
        
        if (iterations >= originalText.length) {
            clearInterval(interval);
            title.textContent = originalText;
        }
        
        iterations += 1/3;
    }, 30);
}

// Efeito de ondas no clique
function createRipple(event) {
    const button = event.currentTarget;
    const circle = document.createElement('span');
    const diameter = Math.max(button.clientWidth, button.clientHeight);
    const radius = diameter / 2;
    
    circle.style.width = circle.style.height = `${diameter}px`;
    circle.style.left = `${event.clientX - button.offsetLeft - radius}px`;
    circle.style.top = `${event.clientY - button.offsetTop - radius}px`;
    circle.classList.add('ripple');
    
    const ripple = button.getElementsByClassName('ripple')[0];
    
    if (ripple) {
        ripple.remove();
    }
    
    button.appendChild(circle);
}

// Adicionar efeito ripple aos botões
document.querySelectorAll('.btn').forEach(button => {
    button.addEventListener('click', createRipple);
});

// CSS para o efeito ripple
const rippleStyle = document.createElement('style');
rippleStyle.textContent = `
    .btn {
        position: relative;
        overflow: hidden;
    }
    
    .ripple {
        position: absolute;
        border-radius: 50%;
        background-color: rgba(255, 255, 255, 0.3);
        transform: scale(0);
        animation: ripple-animation 0.6s linear;
        pointer-events: none;
    }
    
    @keyframes ripple-animation {
        to {
            transform: scale(4);
            opacity: 0;
        }
    }
`;
document.head.appendChild(rippleStyle);

// Efeito de cursor personalizado
function initCustomCursor() {
    const cursor = document.createElement('div');
    cursor.className = 'custom-cursor';
    document.body.appendChild(cursor);
    
    const cursorFollower = document.createElement('div');
    cursorFollower.className = 'cursor-follower';
    document.body.appendChild(cursorFollower);
    
    let mouseX = 0, mouseY = 0;
    let followerX = 0, followerY = 0;
    
    document.addEventListener('mousemove', (e) => {
        mouseX = e.clientX;
        mouseY = e.clientY;
        
        cursor.style.left = mouseX + 'px';
        cursor.style.top = mouseY + 'px';
    });
    
    function animateFollower() {
        followerX += (mouseX - followerX) * 0.1;
        followerY += (mouseY - followerY) * 0.1;
        
        cursorFollower.style.left = followerX + 'px';
        cursorFollower.style.top = followerY + 'px';
        
        requestAnimationFrame(animateFollower);
    }
    
    animateFollower();
    
    // Efeitos hover
    const hoverElements = document.querySelectorAll('a, button, .feature-card, .class-card');
    
    hoverElements.forEach(element => {
        element.addEventListener('mouseenter', () => {
            cursor.classList.add('hover');
            cursorFollower.classList.add('hover');
        });
        
        element.addEventListener('mouseleave', () => {
            cursor.classList.remove('hover');
            cursorFollower.classList.remove('hover');
        });
    });
}

// CSS para cursor personalizado
const cursorStyle = document.createElement('style');
cursorStyle.textContent = `
    .custom-cursor {
        position: fixed;
        width: 10px;
        height: 10px;
        background: var(--primary-color);
        border-radius: 50%;
        pointer-events: none;
        z-index: 9999;
        transform: translate(-50%, -50%);
        transition: transform 0.1s ease;
    }
    
    .cursor-follower {
        position: fixed;
        width: 30px;
        height: 30px;
        border: 2px solid var(--primary-color);
        border-radius: 50%;
        pointer-events: none;
        z-index: 9998;
        transform: translate(-50%, -50%);
        transition: transform 0.3s ease;
    }
    
    .custom-cursor.hover {
        transform: translate(-50%, -50%) scale(1.5);
    }
    
    .cursor-follower.hover {
        transform: translate(-50%, -50%) scale(1.5);
    }
    
    body {
        cursor: none;
    }
    
    a, button {
        cursor: none;
    }
`;
document.head.appendChild(cursorStyle);

// Inicializar cursor personalizado apenas em desktop
if (window.innerWidth > 768) {
    initCustomCursor();
}

// Efeito de loading da página
window.addEventListener('load', function() {
    const loader = document.createElement('div');
    loader.className = 'page-loader';
    loader.innerHTML = `
        <div class="loader-content">
            <div class="loader-logo">Summit Tale</div>
            <div class="loader-bar">
                <div class="loader-progress"></div>
            </div>
        </div>
    `;
    document.body.appendChild(loader);
    
    // Simular carregamento
    let progress = 0;
    const progressBar = loader.querySelector('.loader-progress');
    
    const loadingInterval = setInterval(() => {
        progress += Math.random() * 15;
        if (progress > 100) progress = 100;
        
        progressBar.style.width = progress + '%';
        
        if (progress >= 100) {
            clearInterval(loadingInterval);
            setTimeout(() => {
                loader.style.opacity = '0';
                setTimeout(() => {
                    loader.remove();
                }, 500);
            }, 500);
        }
    }, 100);
});

// CSS para loader
const loaderStyle = document.createElement('style');
loaderStyle.textContent = `
    .page-loader {
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background: var(--secondary-color);
        display: flex;
        align-items: center;
        justify-content: center;
        z-index: 10000;
        transition: opacity 0.5s ease;
    }
    
    .loader-content {
        text-align: center;
        max-width: 300px;
        width: 100%;
    }
    
    .loader-logo {
        font-family: 'Cinzel', serif;
        font-size: 2rem;
        font-weight: 700;
        background: var(--gradient-primary);
        -webkit-background-clip: text;
        -webkit-text-fill-color: transparent;
        background-clip: text;
        margin-bottom: 2rem;
    }
    
    .loader-bar {
        width: 100%;
        height: 4px;
        background: rgba(255, 255, 255, 0.1);
        border-radius: 2px;
        overflow: hidden;
    }
    
    .loader-progress {
        height: 100%;
        background: var(--gradient-primary);
        border-radius: 2px;
        transition: width 0.3s ease;
        width: 0%;
    }
`;
document.head.appendChild(loaderStyle);

// Efeito de neve/partículas mágicas
function createMagicParticles() {
    const particleCount = 30;
    
    for (let i = 0; i < particleCount; i++) {
        const particle = document.createElement('div');
        particle.className = 'magic-particle';
        
        particle.style.left = Math.random() * 100 + '%';
        particle.style.animationDelay = Math.random() * 10 + 's';
        particle.style.animationDuration = (Math.random() * 10 + 10) + 's';
        
        document.body.appendChild(particle);
    }
}

// CSS para partículas mágicas
const magicParticleStyle = document.createElement('style');
magicParticleStyle.textContent = `
    .magic-particle {
        position: fixed;
        top: -10px;
        width: 3px;
        height: 3px;
        background: var(--accent-color);
        border-radius: 50%;
        pointer-events: none;
        z-index: 1;
        animation: fall linear infinite;
        opacity: 0.7;
    }
    
    @keyframes fall {
        0% {
            transform: translateY(-100vh) rotate(0deg);
            opacity: 0;
        }
        10% {
            opacity: 0.7;
        }
        90% {
            opacity: 0.7;
        }
        100% {
            transform: translateY(100vh) rotate(360deg);
            opacity: 0;
        }
    }
`;
document.head.appendChild(magicParticleStyle);

// Inicializar partículas mágicas
createMagicParticles();

// Efeito de shake no hover dos botões principais
document.querySelectorAll('.btn-primary').forEach(button => {
    button.addEventListener('mouseenter', function() {
        this.style.animation = 'shake 0.5s ease-in-out';
    });
    
    button.addEventListener('animationend', function() {
        this.style.animation = '';
    });
});

// CSS para shake
const shakeStyle = document.createElement('style');
shakeStyle.textContent = `
    @keyframes shake {
        0%, 100% { transform: translateX(0); }
        10%, 30%, 50%, 70%, 90% { transform: translateX(-2px); }
        20%, 40%, 60%, 80% { transform: translateX(2px); }
    }
`;
document.head.appendChild(shakeStyle);

