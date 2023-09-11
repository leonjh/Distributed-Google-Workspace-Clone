import { Routes, Route } from 'react-router-dom';
import Login from './Login/Login';
import Register from './Login/Register';
import LandingPage from './LandingPage';
import ErrorPage from './ErrorPage';
import HomePage from './HomePage';
import EmailPage from './EmailPage';
import AdminPage from './AdminPage';

export const Views = () => {

  return (
    <Routes>
      <Route path="/" element={<LandingPage />} />
      <Route path="/login" element={<Login />} />
      <Route path="/register" element={<Register />} />
      <Route path="/home" element={<HomePage />} />
      <Route path="/email" element={<EmailPage />} />
      <Route path="/admin" element={<AdminPage />} />
      <Route path="*" element={<ErrorPage />} />
    </Routes >
  );
};

export default Views;