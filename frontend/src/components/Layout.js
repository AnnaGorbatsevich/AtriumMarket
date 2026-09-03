import React from 'react';
import { NavLink, Outlet } from 'react-router-dom';
import { useAuth } from '../auth';
import { ProfileIcon, CartIcon, HeartIcon, PackageIcon, TagIcon, MessageIcon, BellIcon } from './icons';

const BUYER_NAV = [
  { to: '/profile', label: 'Профиль', icon: ProfileIcon },
  { to: '/cart', label: 'Корзина', icon: CartIcon },
  { to: '/favorites', label: 'Избранное', icon: HeartIcon },
  { to: '/orders', label: 'Мои заказы', icon: PackageIcon },
  { to: '/messages', label: 'Сообщения', icon: MessageIcon },
  { to: '/notifications', label: 'Уведомления', icon: BellIcon },
];

const SELLER_NAV = [
  { to: '/profile', label: 'Профиль', icon: ProfileIcon },
  { to: '/products', label: 'Товары', icon: TagIcon },
  { to: '/orders', label: 'Заказы', icon: PackageIcon },
  { to: '/messages', label: 'Сообщения', icon: MessageIcon },
  { to: '/notifications', label: 'Уведомления', icon: BellIcon },
];

const Layout = () => {
  const { user, logout } = useAuth();
  const navItems = user.role === 'seller' ? SELLER_NAV : BUYER_NAV;

  return (
    <div className="app-shell">
      <header className="topbar">
        <div className="topbar-brand">AtriumMarket</div>
        <nav className="topbar-nav">
          {navItems.map(({ to, label, icon: Icon }) => (
            <NavLink key={to} to={to} className={({ isActive }) => `topbar-link${isActive ? ' active' : ''}`}>
              <Icon />
              <span>{label}</span>
            </NavLink>
          ))}
        </nav>
        <div className="topbar-actions">
          <button type="button" className="topbar-logout" onClick={logout}>
            Выйти
          </button>
        </div>
      </header>
      <main className="app-content">
        <Outlet />
      </main>
    </div>
  );
};

export default Layout;
