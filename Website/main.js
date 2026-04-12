
(function () {
    'use strict';

    /* ===== SCROLL REVEAL ===== */
    const revealObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('visible');
                revealObserver.unobserve(entry.target);
            }
        });
    }, { threshold: 0.08, rootMargin: '0px 0px -36px 0px' });

    document.querySelectorAll('.reveal').forEach(el => revealObserver.observe(el));

    /* ===== STAGGER GRID CHILDREN ===== */
    document.querySelectorAll('.grid-2, .grid-3, .grid-4, .systems-grid, .pattern-detail, .tech-grid, .ref-grid').forEach(grid => {
        Array.from(grid.children).forEach((child, i) => {
            if (child.classList.contains('reveal')) {
                child.style.transitionDelay = (i * 0.09) + 's';
            }
        });
    });

    /* ===== ACTIVE NAV LINK ===== */
    (function setActiveNav() {
        const current = window.location.pathname.split('/').pop() || 'index.html';
        document.querySelectorAll('.nav-links a').forEach(a => {
            const href = a.getAttribute('href');
            if (href === current || (current === '' && href === 'index.html')) {
                a.classList.add('active');
            }
        });
    })();

    /* ===== MOBILE NAV TOGGLE ===== */
    const toggle = document.querySelector('.nav-menu-toggle');
    const navLinks = document.querySelector('.nav-links');

    if (toggle && navLinks) {
        toggle.addEventListener('click', () => {
            navLinks.classList.toggle('open');
        });

        // Close on link click
        navLinks.querySelectorAll('a').forEach(a => {
            a.addEventListener('click', () => navLinks.classList.remove('open'));
        });
    }

    /* ===== BAR FILL ANIMATION on scroll ===== */
    const barObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.querySelectorAll('.bar-fill').forEach(bar => {
                    const target = bar.getAttribute('data-width');
                    if (target) bar.style.width = target;
                });
                barObserver.unobserve(entry.target);
            }
        });
    }, { threshold: 0.2 });

    document.querySelectorAll('.bar-wrap').forEach(wrap => {
        // Initialise widths to 0, store target in data-width
        wrap.querySelectorAll('.bar-fill').forEach(bar => {
            const current = bar.style.width;
            if (current) {
                bar.setAttribute('data-width', current);
                bar.style.width = '0';
            }
        });
        barObserver.observe(wrap);
    });

    /* ===== SMOOTH SCROLL for anchor hrefs ===== */
    document.querySelectorAll('a[href^="#"]').forEach(a => {
        a.addEventListener('click', e => {
            const target = document.querySelector(a.getAttribute('href'));
            if (target) {
                e.preventDefault();
                target.scrollIntoView({ behavior: 'smooth' });
            }
        });
    });

})();
